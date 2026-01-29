
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Terminal;
using Fulfil.Visualization.ErrorLogging;

namespace CvBuilder.Ui;

public class ScriptRunner
{
    public bool IsIdle() => CurrentRunningScript is null;
    public IScript? CurrentRunningScript { get; private set; } = null;
    public TerminalViewModel Terminal { get; }
    public ScriptRunner(TerminalViewModel terminalViewModel)
    {
        Terminal = terminalViewModel;
    }
    public async Task<ScriptCompletionInfo> Run(IScript script)
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
        UserInfo.LogSuccess($"{script.Name} completed successfully");
        return ScriptCompletionInfo.Success;

    }
}
