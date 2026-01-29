using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal abstract class CombinedScript : IScript
{
    public abstract string Name { get; }


    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        foreach (var script in SubSteps())
        {
            var subStepResult = await script.RunAsync(terminal);
            if (!subStepResult.Succeeded)
            {
                return ScriptCompletionInfo.Failure($"sub-step {script.Name} failed: {subStepResult.FailureReason}");
            }
        }
        return ScriptCompletionInfo.Success;
    }
    public abstract IEnumerable<IScript> SubSteps();
}
