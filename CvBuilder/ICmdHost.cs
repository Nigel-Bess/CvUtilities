namespace CvBuilder;

public interface ICmdHost : IDisposable
{
    Action<TerminalOp> OnOp { get; set; }
    void Send(string text);
    void Resize(short cols, short rows);
    int BufferSize { get; }
}

public abstract record TerminalOp
{
    public sealed record AppendText(string Text) : TerminalOp;
    public sealed record SetTitle(string Title) : TerminalOp;
    public sealed record SetCursorVisible(bool Visible) : TerminalOp;

    // Absolute cursor position (VT is 1-based)
    public sealed record SetCursorPos(short Row, short Col) : TerminalOp;

    // Erase-in-display / erase-in-line (ED/EL)
    public sealed record EraseDisplay(EraseDisplayMode Mode) : TerminalOp;
    public sealed record EraseLine(EraseLineMode Mode) : TerminalOp;

    // SGR as raw params for now is fine, but I'd rename to match intent
    public sealed record SetSgr(string Params) : TerminalOp;
}

public enum EraseDisplayMode : byte { ToEnd = 0, ToStart = 1, Entire = 2, Scrollback = 3 }
public enum EraseLineMode : byte { ToEnd = 0, ToStart = 1, Entire = 2 }