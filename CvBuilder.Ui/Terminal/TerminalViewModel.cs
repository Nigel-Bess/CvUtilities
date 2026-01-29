
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
    private readonly ICmdHost _host;
    public TerminalViewModel()
    {
        EnterCommand = new Command(() => Enter());
        _ui = SynchronizationContext.Current ?? new();
        _host = new ConPtyCmdHost();
        _host.OnTextOutput += OnOutputFromCmd;
    }
    private void OnOutputFromCmd(string s)
    {
        AddText(s);
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
        _host.SendCommand(line);
    }

    public async Task<bool> AwaitText(string text, int timeoutMs = 10000)
    {
        if (string.IsNullOrEmpty(text)) return true;

        var cap = _host.BufferSize * 2;
        var tcs = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);
        var tail = new StringBuilder();

        void Handler(string s)
        {
            tail.Append(s);
            if (tail.Length > cap) tail.Remove(0, tail.Length - cap);
            if (tail.ToString().Contains(text, StringComparison.OrdinalIgnoreCase))
                tcs.TrySetResult(true);
        }

        _host.OnTextOutput += Handler;

        try
        {
            var existing = TerminalOutput;
            if (!string.IsNullOrEmpty(existing))
            {
                tail.Append(existing);
                if (tail.Length > cap) tail.Remove(0, tail.Length - cap);
                if (tail.ToString().Contains(text, StringComparison.OrdinalIgnoreCase)) return true;
            }

            return await Task.WhenAny(tcs.Task, Task.Delay(timeoutMs)) == tcs.Task;
        }
        finally
        {
            _host.OnTextOutput -= Handler;
        }
    }

}
