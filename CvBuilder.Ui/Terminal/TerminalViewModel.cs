
using CvBuilder.Ui.Wpf;

namespace CvBuilder.Ui.Terminal;

internal class TerminalViewModel : Notifier
{
    public string TerminalOutput { get => field; set { field = value; NotifyPropertyChanged(); } }
    public string TerminalInput { get; set; }
    public Command EnterCommand { get; }
}
