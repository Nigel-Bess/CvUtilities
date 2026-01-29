
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
    public TerminalViewModel()
    {
        EnterCommand = new AsyncRelayCommand(Enter);
        _ui = SynchronizationContext.Current ?? new();
    }
    private void AddLine(string s) => AddText($"{CurrentDirectoryStr}> {s}\n");
    private void AddText(string s)
    {
        TerminalOutput += $"{s}";
        OnGotText?.Invoke();
    }
    private void Return() => AddText("\n");
    private async Task Enter()
    {
        var line = TerminalInput;
        var directory = CurrentDirectoryStr;
        AddLine(line);
        TerminalInput = "";
        var (errorDode, stdOut, stdErr, newDirectory) = await RunCmd.ExecuteAsync(line, directory);
        AddText(stdOut);
        CurrentDirectoryStr = newDirectory;
        Return();
    }
}
