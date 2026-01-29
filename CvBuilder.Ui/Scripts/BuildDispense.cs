namespace CvBuilder.Ui.Scripts;

internal class BuildDispense : CombinedScript
{
    public override string Name { get; }
    private readonly string _branchName;
    public BuildDispense(string branchName)
    {
        Name = $"Build {branchName}";
    }

    public override IEnumerable<IScript> SubSteps()
    {
        yield return new Ssh(SshLogin.WhismanDab);
        yield return BasicTextCommand.MultiLine([
            "cd code /Fulfil.ComputerVision/",
            "git reset --hard HEAD",
            $"git fetch origin {_branchName}",
            $"git checkout {_branchName}",
            "git pull",
        ]);
    }
}
