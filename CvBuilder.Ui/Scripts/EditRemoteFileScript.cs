using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Terminal;
using CvBuilder.Ui.Util;
using System.IO;
using System.Security.Cryptography;

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
        _localTempFilePath = PathHelpers.GenerateTempFilePath($"{Path.GetFileNameWithoutExtension(remoteFilePath)}_LOCAL_EDIT");
        _remoteFilePath = remoteFilePath;
        if (File.Exists(_localTempFilePath)) File.Delete(_localTempFilePath);
    }

    public override IEnumerable<IScript> SubSteps()
    {
        var sshAuth = new PromptResponse("Auth Ssh", "password:", _ssh.PassWord);
        var hostStr = $"{_ssh.HostName}:\"{_remoteFilePath}\"";
        var localStr = $"\"{_localTempFilePath}\"";
        yield return new BasicTextCommand($"scp {hostStr} {localStr}"); // bring the file to local machine
        yield return sshAuth;
        yield return new GenericScript("Wait for SCP Completion", WaitForLocalFileToExist); // edit the local file instance
        yield return new GenericScript("Backup", MakeABackup);
        yield return new GenericScript("Edit File", EditLocalFile); // edit the local file instance
        yield return new BasicTextCommand($"scp {localStr} {hostStr}"); // copy the local file back to the host        
        yield return sshAuth;
        yield return new GenericScript("Wait for return SCP", WaitForScpCompletionToHost);
        yield return new SshScript(_ssh);
        yield return new BasicTextCommand($"openssl dgst -sha256 {_remoteFilePath}");
        yield return new GenericScript("Wait for SCP to HOST", EnsureCompletionOfScp);
    }

    async Task<ScriptCompletionInfo> MakeABackup(TerminalViewModel terminal)
    {
        var saveDirectory = GetBackupDirectory();
        var now = DateTime.Now;
        var fileName = $"{Path.GetFileNameWithoutExtension(_remoteFilePath)}_Backup_{_ssh.HostName}_{now.Year}-{now.Month}-{now.Day}.{Path.GetExtension(_remoteFilePath)}";
        File.Copy(_localTempFilePath, Path.Combine(saveDirectory, fileName), overwrite: true);
        return ScriptCompletionInfo.Success;
    }

    private string GetBackupDirectory()
    {
        var root = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        var saveLocation = Path.Combine(root, "RemoteFileEditBackups");
        Directory.CreateDirectory(saveLocation);
        return saveLocation;
    }
    async Task<ScriptCompletionInfo> WaitForScpCompletionToHost(TerminalViewModel terminal)
    {
        var localFileName = Path.GetFileName(_localTempFilePath);
        if (!await terminal.AwaitSequentially([localFileName, "100%"])) return ScriptCompletionInfo.Failure("Scp to remote did not complete");
        return ScriptCompletionInfo.Success;
    }
    private string GetLocalFileSha(string localFilePath)
    {
        using var fs = File.OpenRead(localFilePath);
        return Convert.ToHexString(SHA256.HashData(fs)).ToLowerInvariant();
    }
    private async Task<ScriptCompletionInfo> EnsureCompletionOfScp(TerminalViewModel terminal)
    {
        var shaHash = GetLocalFileSha(_localTempFilePath);
        var n = Math.Min(5, shaHash?.Length ?? 0);
        var first5 = shaHash[..n];
        var last5 = shaHash[^n..];
        if (!await terminal.AwaitText("SHA256")) return ScriptCompletionInfo.Failure("Remote host never gave us a sha256 hash");
        if (!(await terminal.AwaitAny([first5, last5])).Success) return ScriptCompletionInfo.Failure($"Remote file sha256 hash did not match {shaHash}"); // look for either the first 5 or last 5 since the hash often gets broken up into parts
        return ScriptCompletionInfo.Success;
    }
    static async Task<bool> WaitForFileAsync(string path, TimeSpan timeout, CancellationToken ct = default)
    {
        if (File.Exists(path)) return true;

        using var timeoutCts = new CancellationTokenSource(timeout);
        using var linkedCts = CancellationTokenSource.CreateLinkedTokenSource(ct, timeoutCts.Token);

        try
        {
            while (!File.Exists(path))
                await Task.Delay(100, linkedCts.Token);

            return true;
        }
        catch (OperationCanceledException) when (timeoutCts.IsCancellationRequested)
        {
            return false;
        }
    }
    private async Task<ScriptCompletionInfo> WaitForLocalFileToExist(TerminalViewModel terminal)
    {

        if (!await WaitForFileAsync(_localTempFilePath, TimeSpan.FromSeconds(2)))
        {
            return ScriptCompletionInfo.Failure("File did not exist after the SCP");
        }
        return ScriptCompletionInfo.Success;
    }
    private async Task<ScriptCompletionInfo> EditLocalFile(TerminalViewModel terminal)
    {
        var fileContents = File.ReadAllText(_localTempFilePath);
        var updated = _applyEdit(fileContents);
        File.WriteAllText(path: _localTempFilePath, contents: updated);
        return ScriptCompletionInfo.Success;
    }

}
