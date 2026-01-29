
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Terminal;

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
    public async Task Run(IScript script)
    {
        if (CurrentRunningScript is not null) throw new InvalidOperationException($"Can not execute {script.Name} while {CurrentRunningScript.Name} is running!");
        CurrentRunningScript = script;
        await script.RunAsync(Terminal);

        CurrentRunningScript = null;
    }
}
