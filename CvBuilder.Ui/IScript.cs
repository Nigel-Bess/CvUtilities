using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui;

public interface IScript
{
    public Task RunAsync(TerminalViewModel terminal);
    public string Name { get; }
}
