using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

public interface IScript
{
    public Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal);
    public string Name { get; }
    /// <summary>
    ///  Reports progress to completion, as a number between 0 and 1. Should be monotonic.
    /// </summary>
    /// <returns></returns>
    public Action<double> ReportProgress { get; set; }
}
