using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class BasicTextCommand : IScript
{
    public string Name => throw new NotImplementedException();
    private readonly string _command;
    private readonly TimeSpan _delay;
    public BasicTextCommand(string command, int delayMs = 100)
    {
        _command = command;
        _delay = TimeSpan.FromMilliseconds(delayMs);
    }

    public async Task RunAsync(TerminalViewModel terminal)
    {
        terminal.Enter(_command);
        await Task.Delay(_delay);
    }

    public static BasicTextCommand MultiLine(List<string> lines) =>
    new(string.Join(" && ", lines));
}
