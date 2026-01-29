namespace CvBuilder;

public interface ICmdHost : IDisposable
{
    public void SendCommand(string text);
    public Action<string> OnTextOutput { get; set; }
    public int BufferSize { get; }
}
