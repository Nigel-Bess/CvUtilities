using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class GenericScript : IScript
{
    public Action<double> ReportProgress { get; set; }
    public delegate Task<ScriptCompletionInfo> ScriptAction(TerminalViewModel terminal);
    public string Name { get; }
    private readonly ScriptAction _action;
    public GenericScript(string name, ScriptAction action)
    {
        Name = name;
        _action = action;
    }

    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        return await _action(terminal);
    }
}
