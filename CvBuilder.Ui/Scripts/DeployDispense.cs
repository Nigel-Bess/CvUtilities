using CvBuilder.Ui.Deploy;
using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class DeployDispense : IScript
{
    public string Name { get; }
    public DeployDispense(DeployableBuild build)
    {

    }

    public Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        throw new NotImplementedException();
    }
}
