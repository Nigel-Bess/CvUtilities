
using CvBuilder.Ui.Wpf;
using System.IO;
using System.Windows.Input;
namespace CvBuilder.Ui.Terminal;

public class TerminalViewModel : Notifier
{
    private readonly CmdHost _cmd;
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
}
