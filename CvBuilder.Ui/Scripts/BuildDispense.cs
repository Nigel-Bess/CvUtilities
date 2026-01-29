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
        yield return new BasicTextCommand("cd code /Fulfil.ComputerVision/");
        yield return new BasicTextCommand($"git reset --hard HEAD");
        yield return new BasicTextCommand($"git fetch origin {_branchName}");
        yield return new BasicTextCommand($"git checkout {_branchName}");
        yield return new BasicTextCommand($"git pull");
    }
}
