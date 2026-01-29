using CvBuilder.Ui.DeployDispense;
using CvBuilder.Ui.Terminal;
using Fulfil.Visualization.ErrorLogging;

namespace CvBuilder.Ui;

public static class Program
{
    public static void Start()
    {
        var terminal = new TerminalViewModel();
        var scriptRunner = new ScriptRunner(terminal);
        var buildAndDeployVm = new BuildAndDeployViewModel(scriptRunner);

        var mainWindowVm = new MainWindowViewModel() { BuildAndDeployVm = buildAndDeployVm };
        var mainWindow = new MainWindow() { DataContext = mainWindowVm };
        UserInfo.LogInfo("Program Started");
        mainWindow.Show();
    }
}
