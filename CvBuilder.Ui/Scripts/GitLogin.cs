using CvBuilder.Ui.Terminal;
using Fulfil.Visualization.ErrorLogging;

namespace CvBuilder.Ui.Scripts;

public class GitLogin : IScript
{
    public string Name { get; } = "Log in to git";

    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        UserInfo.LogInfo("Performing git auth...");
        if (!await terminal.AwaitText("Username for 'https://github.com'")) return ScriptCompletionInfo.Success; // github never asked for a username -> we are already logged in
        var username = UserSettings.Default.GithubUsername;
        var githubPat = UserSettings.Default.GithubPat;
        if (new[] { username, githubPat }.Any(string.IsNullOrWhiteSpace))
        {
            bool success;
            (success, username, githubPat) = PromptForCredentials();
            if (!success) return ScriptCompletionInfo.Failure("Operation cancelled by user");
        }
        terminal.Enter(username);
        if (!(await terminal.AwaitText("Password for"))) return ScriptCompletionInfo.Failure("GH never asked for a password. Something must have gone wrong");
        terminal.Enter(githubPat);
        var failed = await terminal.AwaitText("Invalid username or token", timeoutMs: 1000); // If we see this then login failed
        if (failed)
        {
            return ScriptCompletionInfo.Failure("Incorrect username or password for GH");
        }
        UserInfo.LogInfo("Git login succeeded");
        return ScriptCompletionInfo.Success;
    }

    public static (bool Success, string Username, string Pat) PromptForCredentials()
    {
        var vm = new LoginFormViewModel();
        var form = new LoginForm() { DataContext = vm };
        var window = new OkCancelDialog("Log in to Github", "Login", form) { Width = 300, Height = 200 };
        if (window.ShowDialog() == false) return (false, "", "");
        var username = vm.Username;
        var githubPat = vm.Password;
        UserSettings.Default.GithubUsername = username;
        UserSettings.Default.GithubPat = githubPat;
        UserSettings.Default.Save();
        return (true, username, githubPat);
    }
}
