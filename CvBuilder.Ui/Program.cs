using CvBuilder.Ui.DeployDispense;
using CvBuilder.Ui.Terminal;
using Fulfil.Visualization.ErrorLogging;

namespace CvBuilder.Ui;

public static class Program
{
    public static async void Start()
    {
        var terminal = new TerminalViewModel();
        var scriptRunner = new ScriptRunner(terminal);
        var buildAndDeployVm = new BuildAndDeployViewModel(scriptRunner);

        var mainWindowVm = new MainWindowViewModel() { BuildAndDeployVm = buildAndDeployVm };
        var mainWindow = new MainWindow() { DataContext = mainWindowVm };
        UserInfo.LogInfo("Program Started");
        if (Slack.TryGetToken(out var token))
        {
            var users = await SlackHelpers.GetLockEmojiHoldersAsync(channelId: "C071ZQT4VP1", messageTimeStamp: "1759344188.391349", slackToken: token);
            foreach (var user in users) UserInfo.LogSuccess(user);
        }

        mainWindow.Show();
    }
}
