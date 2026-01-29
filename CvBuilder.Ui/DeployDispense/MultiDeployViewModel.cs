namespace CvBuilder.Ui.DeployDispense;

public class MultiDeployViewModel
{
    public List<ScriptRunner> Runners { get; }

    public MultiDeployViewModel(IEnumerable<ScriptRunner> runners)
    {
        Runners = runners.ToList();
    }
}
