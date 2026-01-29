using CvBuilder.Ui.Terminal;

namespace CvBuilder.Ui.Scripts;

internal class PromptResponse : IScript
{
    public string Name { get; }
    private readonly string _prompt;
    private readonly string _response;

    public PromptResponse(string name, string prompt, string response)
    {
        Name = name;
        _prompt = prompt;
        _response = response;
    }
    public async Task<ScriptCompletionInfo> RunAsync(TerminalViewModel terminal)
    {
        if (!await terminal.AwaitText(_prompt)) return ScriptCompletionInfo.Failure($"we were never prompted for '{_prompt}'");
        terminal.Enter(_response);
        return ScriptCompletionInfo.Success;
    }
}
