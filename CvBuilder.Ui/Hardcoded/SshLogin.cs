namespace CvBuilder.Ui.Hardcoded;

public record SshLogin(string HostName, string PassWord)
{
    public static SshLogin WhismanDab => new("fulfil@clip-tb.whisman.fulfil.ai", "FreshEngr");
}
