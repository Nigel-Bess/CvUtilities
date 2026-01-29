using CvBuilder.Ui.Terminal;
using System.Windows;

namespace CvBuilder.Ui.Scripts;

public class GitLogin : IScript
{
    public string Name { get; } = "Log in to git";

    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        if (!await terminal.AwaitText("Username for 'https://github.com'")) return ScriptCompletionInfo.Success; // github never asked for a username -> we are already logged in
        var username = UserSettings.Default.GithubUsername;
        var githubPat = UserSettings.Default.GithubPat;
        if (new[] { username, githubPat }.Any(string.IsNullOrWhiteSpace))
        {

            var vm = new LoginFormViewModel();
            var form = new LoginForm() { DataContext = vm };
            var window = new Window() { Content = form, Width = 300, Height = 200 };
            if (window.ShowDialog() == false) return ScriptCompletionInfo.Failure("Operation cancelled by user");
            username = vm.Username;
            githubPat = vm.Password;
            UserSettings.Default.GithubUsername = username;
            UserSettings.Default.GithubPat = githubPat;
            UserSettings.Default.Save();
        }
        terminal.Enter(username);
        if (!(await terminal.AwaitText("Password for"))) return ScriptCompletionInfo.Failure("GH never asked for a password. Something must have gone wrong");
        terminal.Enter(githubPat);
        await Task.Delay(1000);
        return ScriptCompletionInfo.Success;
    }
}
