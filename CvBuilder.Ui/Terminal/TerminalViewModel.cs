
using CvBuilder.Ui.Wpf;
using Fulfil.Visualization.ErrorLogging;
using System.Collections.Concurrent;
using System.IO;
using System.Text;
using System.Windows.Input;
namespace CvBuilder.Ui.Terminal;

public class TerminalViewModel : Notifier
{
    private readonly SynchronizationContext _ui;
    public string TerminalInput { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public string CurrentDirectoryStr { get => field; set { field = value; NotifyPropertyChanged(); } } = Directory.GetCurrentDirectory();
    public ICommand EnterCommand { get; }
    private ICmdHost _host;
    private StringBuilder _textSinceLastCommand { get; } = new();
    private bool ConsoleHasStarted = false;

    /// <summary>
    /// Raised on the UI thread when output should be cleared (e.g. on Reset, or when trimming output).
    /// </summary>
    public event Action? OutputReset;

    /// <summary>
    /// Raised on the UI thread with output to append.
    /// </summary>
    public event Action<string>? OutputAppended;

    private readonly ConcurrentQueue<string> _pendingOutput = new();
    private int _flushScheduled;
    private readonly StringBuilder _rollingOutput = new();
    public int MaxOutputChars { get; set; } = 200_000;

    public TerminalViewModel()
    {
        EnterCommand = new Command(() => Enter());
        _ui = SynchronizationContext.Current ?? new();
        Reset();
    }
    private void OnOutputFromCmd(TerminalOp operation)
    {
        if (operation is not TerminalOp.AppendText append) return;
        ConsoleHasStarted = true;
        var s = append.Text;
        AddText(s);
        _textSinceLastCommand.Append(s);
    }
    public void Reset(string message = "")
    {
        ConsoleHasStarted = false;
        _host?.OnOp -= OnOutputFromCmd;
        _host?.Dispose();
        _host = new ConPtyCmdHost();
        _host.OnOp += OnOutputFromCmd;

        ClearPendingOutput();
        ClearOutput();
        if (!string.IsNullOrEmpty(message)) AddText(message);
    }
    private void AddLine(string s) => AddText($"{CurrentDirectoryStr}> {s}\n");

    private void Return() => AddText("\n");

    void AddText(string s)
    {
        if (string.IsNullOrEmpty(s)) return;
        _pendingOutput.Enqueue(s);

        if (Interlocked.Exchange(ref _flushScheduled, 1) == 0)
        {
            _ui.Post(_ => FlushPendingOutputOnUi(), null);
        }
    }

    void FlushPendingOutputOnUi()
    {
        try
        {
            if (_pendingOutput.IsEmpty) return;

            var batch = new StringBuilder();
            while (_pendingOutput.TryDequeue(out var part))
                batch.Append(part);

            if (batch.Length == 0) return;

            var trimmed = false;
            _rollingOutput.Append(batch);
            if (_rollingOutput.Length > MaxOutputChars && MaxOutputChars > 0)
            {
                _rollingOutput.Remove(0, _rollingOutput.Length - MaxOutputChars);
                trimmed = true;
            }

            if (trimmed)
            {
                OutputReset?.Invoke();
                OutputAppended?.Invoke(_rollingOutput.ToString());
            }
            else
            {
                OutputAppended?.Invoke(batch.ToString());
            }
        }
        finally
        {
            Interlocked.Exchange(ref _flushScheduled, 0);
            if (!_pendingOutput.IsEmpty && Interlocked.Exchange(ref _flushScheduled, 1) == 0)
                _ui.Post(_ => FlushPendingOutputOnUi(), null);
        }
    }

    void ClearPendingOutput()
    {
        while (_pendingOutput.TryDequeue(out _)) { }
        _rollingOutput.Clear();
        Interlocked.Exchange(ref _flushScheduled, 0);
    }

    void ClearOutput() => _ui.Post(_ => OutputReset?.Invoke(), null);

    public void Enter(string? line = null)
    {
        line ??= TerminalInput;
        TerminalInput = "";
        AddLine(line);
        Return();
        _textSinceLastCommand.Clear();
        _host.Send(line);
    }
    public async Task<bool> AwaitConsoleStartup(int timeoutMs = 100)
    {
        if (ConsoleHasStarted) return true;
        if (_textSinceLastCommand.Length > 0) return ConsoleHasStarted = true;

        var tcs = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);

        void Handler(TerminalOp op)
        {
            if (op is TerminalOp.AppendText a && a.Text.Length > 0)
                tcs.TrySetResult(true);
        }

        _host.OnOp += Handler;
        try
        {
            var ok = await Task.WhenAny(tcs.Task, Task.Delay(timeoutMs)) == tcs.Task;
            return ConsoleHasStarted = ok;
        }
        finally
        {
            _host.OnOp -= Handler;
        }
    }

    public async Task<bool> AwaitSequentially(IEnumerable<string> toFind, int timeoutMs = 10000)
    {
        var tasks = new List<Task<bool>>();
        foreach (var (idx, str) in toFind.Index())
        {
            if (!await AwaitText(str, timeoutMs: timeoutMs, inclueTextSinceLastCommand: idx == 0)) return false;
            UserInfo.LogInfo($"Found {str}");
        }
        return true;
    }

    public async Task<bool> AwaitAll(IEnumerable<string> toFind, int timeoutMs = 10000)
    {
        var tasks = new List<Task<bool>>();
        foreach (var str in toFind)
        {
            tasks.Add(AwaitText(str, timeoutMs));
        }
        foreach (var t in tasks)
        {
            if (!await t) return false;
        }
        return true;
    }

    public async Task<(bool Success, string? First)> AwaitAny(IEnumerable<string> toFind, int timeoutMs = 10000)
    {
        var tasks = new Dictionary<Task<bool>, string>();
        foreach (var str in toFind)
        {
            tasks[AwaitText(str, timeoutMs)] = str;
        }
        var first = await Task.WhenAny(tasks.Keys);
        var success = await first;
        return (success, success ? tasks[first] : null);
    }

    public async Task<bool> AwaitText(string text, int timeoutMs = 10000, bool inclueTextSinceLastCommand = true)
    {
        if (string.IsNullOrEmpty(text)) return true;
        if (inclueTextSinceLastCommand)
        {
            var textSinceLastCommand = _textSinceLastCommand.ToString();
            if (textSinceLastCommand.Contains(text, StringComparison.OrdinalIgnoreCase)) return true;
        }

        var cap = _host.BufferSize * 2;
        var tcs = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);
        var tail = new StringBuilder();

        void Handler(TerminalOp operation)
        {
            if (operation is not TerminalOp.AppendText append) return;
            tail.Append(append.Text);
            if (tail.Length > cap) tail.Remove(0, tail.Length - cap);
            if (tail.ToString().Contains(text, StringComparison.OrdinalIgnoreCase))
                tcs.TrySetResult(true);
        }

        _host.OnOp += Handler;

        try
        {
            return await Task.WhenAny(tcs.Task, Task.Delay(timeoutMs)) == tcs.Task;
        }
        finally
        {
            _host.OnOp -= Handler;
        }
    }

}
