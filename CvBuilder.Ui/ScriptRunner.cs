
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Terminal;
using Fulfil.Visualization.ErrorLogging;

namespace CvBuilder.Ui;

public class ScriptRunner
{
    public bool IsIdle() => CurrentRunningScript is null;
    public IScript? CurrentRunningScript { get; private set; } = null;
    public TerminalViewModel Terminal { get; }
    public ScriptRunReportViewModel ReportViewModel { get; } = new();
    public ScriptRunner(TerminalViewModel terminalViewModel)
    {
        Terminal = terminalViewModel;
    }
    public async Task<ScriptCompletionInfo> Run(IScript script)
    {
        ReportViewModel.Start(script);
        var result = await RunInternal(script);
        if (!result.Succeeded)
        {
            UserInfo.LogError($"{script.Name} Failed: {result.FailureReason}");
        }
        UserInfo.LogSuccess($"{script.Name} completed successfully");
        ReportViewModel.End(result);
        return result;
    }

    public async Task<ScriptCompletionInfo> RunInternal(IScript script)
    {
        OnGotProgress(0);
        script.ReportProgress += OnGotProgress;
        try
        {
            Terminal.Reset($"Running {script.Name}");
            if (CurrentRunningScript is not null)
            {
                return ScriptCompletionInfo.Failure($"Can not execute {script.Name} while {CurrentRunningScript.Name} is running!");
            }
            UserInfo.LogInfo($"Running script: {script.Name}...");
            CurrentRunningScript = script;
            var result = await script.RunAsync(Terminal);
            CurrentRunningScript = null;
            if (!result.Succeeded)
            {
                return ScriptCompletionInfo.Failure($"{script.Name} failed! {result.FailureReason}");
            }
            OnGotProgress(100);
            return ScriptCompletionInfo.Success;
        }
        finally
        {
            script.ReportProgress -= OnGotProgress;
        }
    }



    private void OnGotProgress(double progress)
    {
        ReportViewModel.Progress = progress;
    }
}
