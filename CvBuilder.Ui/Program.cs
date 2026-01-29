using CvBuilder.Ui.Deploy;
using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui;

public static class Program
{
    public static void Start()
    {
        var terminal = new TerminalViewModel();
        var buildAndDeployVm = new BuildAndDeployViewModel() { TerminalVm = terminal };

        var mainWindowVm = new MainWindowViewModel() { BuildAndDeployVm = buildAndDeployVm };
        var mainWindow = new MainWindow() { DataContext = mainWindowVm };
        mainWindow.Show();
    }
}
