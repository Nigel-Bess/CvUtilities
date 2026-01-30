using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Terminal;
using CvBuilder.Ui.Util;
using System.IO;

namespace CvBuilder.Ui.Scripts;

public class EditRemoteFileScript : CombinedScript
{
    public override string Name { get; }
    public Action<double> ReportProgress { get; set; }
    private readonly string _localTempFilePath;
    private readonly string _remoteFilePath;
    private readonly SshLogin _ssh;
    private readonly Func<string, string> _applyEdit;
    public EditRemoteFileScript(string name, SshLogin ssh, string remoteFilePath, Func<string, string> applyEdit)
    {
        Name = name;
        _ssh = ssh;
        _applyEdit = applyEdit;
        _localTempFilePath = PathHelpers.GenerateTempFilePath($"{Name}_LOCAL_EDIT");
        _remoteFilePath = remoteFilePath;
        if (File.Exists(_localTempFilePath)) File.Delete(_localTempFilePath);
    }

    public override IEnumerable<IScript> SubSteps()
    {
        var sshAuth = () => new PromptResponse("Auth Ssh", "password:", _ssh.PassWord);
        yield return new BasicTextCommand($"scp {_ssh.HostName}:\"{_remoteFilePath}\" \"{_localTempFilePath}\""); // bring the file to local machine
        yield return sshAuth();
        yield return new GenericScript("Edit File", EditLocalFile); // edit the local file instance
        yield return new BasicTextCommand($"scp \"{_localTempFilePath}\" {_ssh.HostName}:/{_remoteFilePath}"); // copy the local file back to the host
        yield return sshAuth();
    }

    private async Task<ScriptCompletionInfo> EditLocalFile(TerminalViewModel terminal)
    {
        if (!File.Exists(_localTempFilePath))
        {
            return ScriptCompletionInfo.Failure("File did not exist after the SCP");
        }
        var fileContents = File.ReadAllText(_localTempFilePath);
        var updated = _applyEdit(fileContents);
        File.WriteAllText(path: _localTempFilePath, contents: updated);
        return ScriptCompletionInfo.Success;
    }

}
