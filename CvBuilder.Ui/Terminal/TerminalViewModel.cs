
using CvBuilder.Ui.Wpf;

namespace CvBuilder.Ui.Terminal;

public class TerminalViewModel : Notifier
{
    public string TerminalOutput { get => field; set { field = value; NotifyPropertyChanged(); } } = "yoooo";
    public string TerminalInput { get; set; }
    public Command EnterCommand { get; }
    public TerminalViewModel()
    {
        EnterCommand = new(Enter);
    }

    private void Enter()
    {
        TerminalOutput += $"\n{TerminalInput}";
    }
}
