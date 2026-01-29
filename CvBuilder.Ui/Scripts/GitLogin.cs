using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

public class GitLogin : IScript
{
    public string Name { get; } = "Log in to git";

    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        if (!await terminal.AwaitText("'https://github.com': ")) return ScriptCompletionInfo.Success; // github never asked for a username -> we are already logged in
        return ScriptCompletionInfo.Success;
    }
}
