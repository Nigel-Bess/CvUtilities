namespace CvBuilder.Ui.Deploy;

public class BuildAndDeployViewModel
{
    public ScriptRunner ScriptRunner { get; }
    public BuildAndDeployViewModel(ScriptRunner scriptRunner)
    {
        ScriptRunner = scriptRunner;
    }
}
