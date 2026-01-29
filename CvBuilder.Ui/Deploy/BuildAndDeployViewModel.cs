using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Deploy;

public class BuildAndDeployViewModel
{
    public TerminalViewModel TerminalVm { get; }
    private readonly ScriptRunner _scriptRunner;
    public BuildAndDeployViewModel(TerminalViewModel terminalVm, ScriptRunner scriptRunner)
    {
        TerminalVm = terminalVm;
        _scriptRunner = scriptRunner;

    }
}
