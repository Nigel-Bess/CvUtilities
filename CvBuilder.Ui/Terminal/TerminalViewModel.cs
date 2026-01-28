
using CvBuilder.Ui.Wpf;
namespace CvBuilder.Ui.Terminal;

public class TerminalViewModel : Notifier
{
    private readonly CmdHost _cmd;
    private readonly SynchronizationContext _ui;
    public string TerminalOutput { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public string TerminalInput { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public string CurrentDirectoryStr { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public Command EnterCommand { get; }
    public TerminalViewModel()
    {
        EnterCommand = new(Enter);
        _ui = SynchronizationContext.Current ?? new();
        _cmd = new CmdHost();
    }


    private void OnDirectoryChanged(string dir) => CurrentDirectoryStr = dir;
    private void AddLine(string s) => TerminalOutput += $"{s}\n";
    private void AddText(string s) => TerminalOutput += $"{s}";

    private void Enter()
    {
        var line = TerminalInput;
        if (string.IsNullOrWhiteSpace(line)) return;
        TerminalInput = "";
        AddLine($"{CurrentDirectoryStr}> {line}");
        _cmd.Execute(line);
    }
}
