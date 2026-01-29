using CvBuilder.Ui.Terminal;

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

    public async Task RunAsync(TerminalViewModel terminal)
    {
        terminal.Enter($"ssh {_sshLogin.HostName}");
        await terminal.AwaitText("password:");
        terminal.Enter(_sshLogin.PassWord);
    }
}
