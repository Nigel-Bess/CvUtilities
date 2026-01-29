using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class BasicTextCommand : IScript
{
    public Action<double> ReportProgress { get; set; }
    public string Name { get; }
    private readonly string _command;
    private readonly TimeSpan _delay;
    public BasicTextCommand(string command, int delayMs = 100)
    {
        _command = command;
        _delay = TimeSpan.FromMilliseconds(delayMs);
        Name = command;
    }

    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        terminal.Enter(_command);
        await Task.Delay(_delay);
        return ScriptCompletionInfo.Success;
    }

    public static BasicTextCommand MultiLine(List<string> lines) =>
    new(string.Join(" && ", lines));
}
