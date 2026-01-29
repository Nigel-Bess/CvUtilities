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
        yield return new GenericScript("Wait for build to finish", WaitForBuildToFinish);
    }

    private async Task<ScriptCompletionInfo> WaitForBuildToFinish(TerminalViewModel terminal)
    {
        const string FailureText = "Build Failed";//TODO
        const string SuccessText = "Build Complete"; // TODO
        var waitForFailure = terminal.AwaitText(FailureText);
        var waitForSuccess = terminal.AwaitText(SuccessText);
        var winner = await Task.WhenAny([waitForFailure, waitForSuccess]);
        if (winner == waitForFailure) return ScriptCompletionInfo.Failure("Build Failed");
        if (winner == waitForSuccess) return ScriptCompletionInfo.Success;
        return ScriptCompletionInfo.Failure("Unknown error occurred. It is unclear if the build completed or failed. Assuming build failed.");
    }

}
