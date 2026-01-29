using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Terminal;
using Fulfil.Visualization.ErrorLogging;

namespace CvBuilder.Ui.Scripts;

internal class Ssh : IScript
{
    public string Name { get; }
    private readonly SshLogin _sshLogin;
    public Ssh(SshLogin sshLogin)
    {
        _sshLogin = sshLogin;
        Name = $"SSH {sshLogin.HostName}";
    }

    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        var host = _sshLogin.HostName;
        UserInfo.LogInfo($"SSHing into {host}");
        terminal.Enter($"ssh {host}");
        if (!await terminal.AwaitText("password:"))
        {
            return ScriptCompletionInfo.Failure($"{host} never asked for a password.");
        }

        terminal.Enter(_sshLogin.PassWord);
        if (!await terminal.AwaitText("Welcome"))
        {
            return ScriptCompletionInfo.Failure($"Incorrect password when sshing into {host}");
        }
        await terminal.AwaitText("Welcome");
        await Task.Delay(100);
        UserInfo.LogInfo($"SSHed into {host}");
        return ScriptCompletionInfo.Success;
    }
}
