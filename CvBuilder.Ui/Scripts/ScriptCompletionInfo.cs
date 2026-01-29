namespace CvBuilder.Ui.Scripts;

public class ScriptCompletionInfo
{
    public required bool Succeeded { get; init; }
    public required string? FailureReason { get; init; }
    public static ScriptCompletionInfo Success => new() { FailureReason = null, Succeeded = true };
    public static ScriptCompletionInfo Failure(string reason) => new() { FailureReason = reason, Succeeded = false };
}
