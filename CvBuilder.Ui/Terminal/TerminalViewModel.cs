
using CvBuilder.Ui.Wpf;
using System.IO;
using System.Text;
using System.Windows.Input;
namespace CvBuilder.Ui.Terminal;

public class TerminalViewModel : Notifier
{
    private readonly SynchronizationContext _ui;
    public Action OnGotText { get; set; }
    public string TerminalOutput { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public string TerminalInput { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public string CurrentDirectoryStr { get => field; set { field = value; NotifyPropertyChanged(); } } = Directory.GetCurrentDirectory();
    public ICommand EnterCommand { get; }
    private ICmdHost _host;
    private StringBuilder _textSinceLastCommand { get; } = new();
    public TerminalViewModel()
    {
        EnterCommand = new Command(() => Enter());
        _ui = SynchronizationContext.Current ?? new();
        Reset();
    }
    private void OnOutputFromCmd(TerminalOp operation)
    {
        if (operation is not TerminalOp.AppendText append) return;
        var s = append.Text;
        AddText(s);
        _textSinceLastCommand.Append(s);
    }
    public void Reset(string message = "")
    {
        _host?.OnOp -= OnOutputFromCmd;
        _host?.Dispose();
        _host = new ConPtyCmdHost();
        _host.OnOp += OnOutputFromCmd;
        TerminalOutput = message;
    }
    private void AddLine(string s) => AddText($"{CurrentDirectoryStr}> {s}\n");

    private void Return() => AddText("\n");
    void AddText(string s) =>
     _ui.Post(_ =>
     {
         TerminalOutput += s;
         OnGotText?.Invoke();
     }, null);

    public void Enter(string? line = null)
    {
        line ??= TerminalInput;
        TerminalInput = "";
        AddLine(line);
        Return();
        _textSinceLastCommand.Clear();
        _host.Send(line);
    }
    public async Task<bool> AwaitSequentially(IEnumerable<string> toFind, int timeoutMs = 10000)
    {
        var tasks = new List<Task<bool>>();
        foreach (var (idx, str) in toFind.Index())
        {
            if (!await AwaitText(str, timeoutMs: timeoutMs, inclueTextSinceLastCommand: idx == 0)) return false;
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
