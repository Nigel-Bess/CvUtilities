using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class BuildDispense : CombinedScript
{
    public override string Name { get; }
    private readonly string _branchName;
    private readonly string _facilityName;
    public BuildDispense(string branchName, string facilityName)
    {
        Name = $"Build {branchName}";
        _branchName = branchName;
        _facilityName = facilityName;
    }

    public override IEnumerable<IScript> SubSteps()
    {
        yield return new Ssh(SshLogin.WhismanDab);
        yield return BasicTextCommand.MultiLine([
            "cd ~/code/Fulfil.ComputerVision/",
            "git reset --hard HEAD",
            $"git fetch origin {_branchName}",
        ]);
        yield return new GitLogin();
        yield return BasicTextCommand.MultiLine([
            $"git checkout {_branchName}",
            "git pull",
        ]);
        yield return new GitLogin();
        yield return new BasicTextCommand("bash /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/scripts/dab-push-latest.sh");
        yield return new PromptResponse(name: "Input branch name", prompt: "Enter Branch Name", response: _branchName);
        yield return new PromptResponse(name: "Input facility name", prompt: "Enter Facility Name", response: _facilityName);
        yield return new GitLogin();
        yield return new GenericScript("Wait for build to finish", WaitForBuildToFinish);
    }

    private async Task<ScriptCompletionInfo> WaitForBuildToFinish(TerminalViewModel terminal)
    {
        var maxTimeMs = (int)TimeSpan.FromMinutes(20).TotalMilliseconds;
        var buildSuccess = terminal.AwaitAll(["Pushing [==================================================>]", "code/Fulfil.ComputerVision"], maxTimeMs);
        var alreadyExisted = terminal.AwaitAll(["Layer already exists", "code/Fulfil.ComputerVision"], maxTimeMs);
        var buildFailure = terminal.AwaitText("did not complete successfully", maxTimeMs);
        var firstToComplete = await Task.WhenAny([buildFailure, buildSuccess, alreadyExisted]);
        if (!await firstToComplete) return ScriptCompletionInfo.Failure("Build timed out");
        if (firstToComplete == buildFailure) return ScriptCompletionInfo.Failure("Build failed!");
        return ScriptCompletionInfo.Success;
    }

}
