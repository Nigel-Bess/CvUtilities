using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

public interface IScript
{
    public Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal);
    public string Name { get; }
}
