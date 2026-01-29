using CvBuilder.Ui.Deploy;
using CvBuilder.Ui.DeployDispense;
using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class DeployDispenseScript : IScript
{
    public string Name { get; }
    public DeployDispenseScript(DeployableBuild build, Dispense dispense)
    {
        Name = $"Deploy {build} to {dispense}";
    }

    public Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        throw new NotImplementedException();
    }
}
