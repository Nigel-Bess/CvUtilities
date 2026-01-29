using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class MockScript : IScript
{
    public string Name { get; }

    public Action<double> ReportProgress { get; set; }
    private readonly int _repeats;
    private readonly TimeSpan _interval;
    public MockScript(double completionTimeSeconds = 20, double updateIntervalMs = 100)
    {
        _repeats = (int)(completionTimeSeconds * 1000 / updateIntervalMs);
        _interval = TimeSpan.FromMilliseconds(updateIntervalMs);
    }

    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        for (var i = 0; i < _repeats; i++)
        {
            terminal.Enter("dir");
            await Task.Delay(_interval);
            ReportProgress?.Invoke(((double)i) / _repeats);
        }
        return ScriptCompletionInfo.Success;
    }
}
