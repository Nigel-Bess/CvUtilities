using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal abstract class CombinedScript : IScript
{
    public Action<double> ReportProgress { get; set; }
    public abstract string Name { get; }


    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        var subSteps = SubSteps().ToList();
        var stepProgress = new Dictionary<IScript, double>();
        foreach (var script in subSteps) stepProgress[script] = 0;
        void OnGotProgress(IScript script, double subscriptProgress)
        {
            stepProgress[script] += subscriptProgress / subSteps.Count;
            ReportProgress?.Invoke(stepProgress.Values.Sum());
        }
        foreach (var script in subSteps)
        {
            var progressReport = (double progress) => OnGotProgress(script, progress);
            script.ReportProgress += progressReport;
            var subStepResult = await script.RunAsync(terminal);
            progressReport(1);
            script.ReportProgress -= progressReport;
            if (!subStepResult.Succeeded)
            {
                return ScriptCompletionInfo.Failure($"sub-step {script.Name} failed: {subStepResult.FailureReason}");
            }
        }
        return ScriptCompletionInfo.Success;
    }
    public abstract IEnumerable<IScript> SubSteps();
}
