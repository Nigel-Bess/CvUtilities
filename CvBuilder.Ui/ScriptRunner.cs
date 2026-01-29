
using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui;

public class ScriptRunner
{
    public IScript? CurrentRunningScript { get; private set; } = null;
    private readonly TerminalViewModel _terminalViewModel;
    public ScriptRunner(TerminalViewModel terminalViewModel)
    {
        _terminalViewModel = terminalViewModel;
    }
    public async Task Run(IScript script)
    {
        if (CurrentRunningScript is not null) throw new InvalidOperationException($"Can not execute {script.Name} while {CurrentRunningScript.Name} is running!");
        CurrentRunningScript = script;
        await script.RunAsync(_terminalViewModel);

        CurrentRunningScript = null;
    }
}
