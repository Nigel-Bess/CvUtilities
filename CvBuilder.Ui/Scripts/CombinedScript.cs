using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal abstract class CombinedScript : IScript
{
    public abstract string Name { get; }


    public async Task RunAsync(TerminalViewModel terminal)
    {
        foreach (var script in SubSteps()) await script.RunAsync(terminal);
    }
    public abstract IEnumerable<IScript> SubSteps();
}
