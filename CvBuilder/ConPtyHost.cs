using Microsoft.Win32.SafeHandles;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Text;

namespace CvBuilder;

public sealed class ConPtyCmdHost : ICmdHost
{
    public int BufferSize { get; } = 8192;
    public Action<TerminalOp> OnOp { get; set; } = _ => { };

    readonly Stream _in;
    readonly Stream _out;
    readonly IntPtr _hpc;
    readonly SafeFileHandle _proc;
    readonly CancellationTokenSource _cts = new();

    readonly Decoder _decoder = Encoding.UTF8.GetDecoder();
    readonly char[] _chars = new char[8192];

    enum ParseState : byte { Text, Esc, Csi, Osc, OscEsc }
    ParseState _state;
    readonly StringBuilder _text = new();
    readonly StringBuilder _seq = new();

    public ConPtyCmdHost(string commandLine = "cmd.exe", short cols = 120, short rows = 30)
    {
        var sa = new SECURITY_ATTRIBUTES { nLength = Marshal.SizeOf<SECURITY_ATTRIBUTES>(), bInheritHandle = false };

        CreatePipe(out var inRead, out var inWrite, ref sa, 0);
        CreatePipe(out var outRead, out var outWrite, ref sa, 0);

        var hr = CreatePseudoConsole(new COORD { X = cols, Y = rows }, inRead, outWrite, 0, out _hpc);
        if (hr != 0) throw new Win32Exception(hr);

        inRead.Dispose();
        outWrite.Dispose();

        var attrSize = IntPtr.Zero;
        InitializeProcThreadAttributeList(IntPtr.Zero, 1, 0, ref attrSize);
        var attrList = Marshal.AllocHGlobal(attrSize);

        try
        {
            if (!InitializeProcThreadAttributeList(attrList, 1, 0, ref attrSize)) throw new Win32Exception(Marshal.GetLastWin32Error());
            if (!UpdateProcThreadAttribute(attrList, 0, (IntPtr)PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, _hpc, (IntPtr)IntPtr.Size, IntPtr.Zero, IntPtr.Zero))
                throw new Win32Exception(Marshal.GetLastWin32Error());

            var si = new STARTUPINFOEX { StartupInfo = new STARTUPINFO { cb = Marshal.SizeOf<STARTUPINFOEX>() }, lpAttributeList = attrList };
            if (!CreateProcessW(null, commandLine, IntPtr.Zero, IntPtr.Zero, true, EXTENDED_STARTUPINFO_PRESENT, IntPtr.Zero, null, ref si, out var pi))
                throw new Win32Exception(Marshal.GetLastWin32Error());

            _proc = new SafeFileHandle(pi.hProcess, true);
            CloseHandle(pi.hThread);

            _in = new FileStream(inWrite, FileAccess.Write, 0, false);
            _out = new FileStream(outRead, FileAccess.Read, 0, false);

            _ = PumpOutputAsync(_cts.Token);
        }
        finally
        {
            DeleteProcThreadAttributeList(attrList);
            Marshal.FreeHGlobal(attrList);
        }
    }

    public void Send(string text)
    {
        if (_cts.IsCancellationRequested) return;

        var s = text.EndsWith('\n') || text.EndsWith('\r') ? text : text + "\r";
        var bytes = Encoding.UTF8.GetBytes(s);

        _in.Flush();
        _in.Write(bytes, 0, bytes.Length);
        _in.Flush();
    }

    public void Resize(short cols, short rows)
    {
        var hr = ResizePseudoConsole(_hpc, new COORD { X = cols, Y = rows });
        if (hr != 0) throw new Win32Exception(hr);
    }

    public void Dispose()
    {
        _cts.Cancel();
        try { _out.Dispose(); } catch { }
        try { _in.Dispose(); } catch { }
        try { _proc.Dispose(); } catch { }
        try { ClosePseudoConsole(_hpc); } catch { }
        _cts.Dispose();
    }

    async Task PumpOutputAsync(CancellationToken ct)
    {
        var buf = new byte[BufferSize];

        while (!ct.IsCancellationRequested)
        {
            int n;
            try { n = await _out.ReadAsync(buf.AsMemory(0, buf.Length), ct); }
            catch { break; }
            if (n <= 0) break;

            _decoder.Convert(buf, 0, n, _chars, 0, _chars.Length, false, out _, out var charsUsed, out _);
            Process(_chars.AsSpan(0, charsUsed));
            FlushText();
        }

        FlushText();
    }

    void Process(ReadOnlySpan<char> s)
    {
        for (var i = 0; i < s.Length; i++)
        {
            var ch = s[i];

            switch (_state)
            {
                case ParseState.Text:
                    if (ch == '\x1b') { _state = ParseState.Esc; break; }
                    _text.Append(ch);
                    break;

                case ParseState.Esc:
                    if (ch == '[') { _seq.Clear(); _state = ParseState.Csi; break; }
                    if (ch == ']') { _seq.Clear(); _state = ParseState.Osc; break; }
                    _state = ParseState.Text; // unknown ESC sequence
                    break;

                case ParseState.Csi:
                    if (ch is >= '\x40' and <= '\x7e')
                    {
                        HandleCsi(ch, _seq.ToString());
                        _state = ParseState.Text;
                    }
                    else _seq.Append(ch);
                    break;

                case ParseState.Osc:
                    if (ch == '\x07') { HandleOsc(_seq.ToString()); _state = ParseState.Text; break; } // BEL
                    if (ch == '\x1b') { _state = ParseState.OscEsc; break; }
                    _seq.Append(ch);
                    break;

                case ParseState.OscEsc:
                    if (ch == '\\') HandleOsc(_seq.ToString()); // ST
                    _state = ParseState.Text;
                    break;
            }
        }
    }

    void HandleOsc(string payload)
    {
        // "0;title" or "2;title" (xterm)
        var semi = payload.IndexOf(';');
        if (semi <= 0) return;

        if (payload.AsSpan(0, semi) is var code && (code.SequenceEqual("0".AsSpan()) || code.SequenceEqual("2".AsSpan())))
        {
            FlushText();
            OnOp(new TerminalOp.SetTitle(payload[(semi + 1)..]));
        }
    }

    void HandleCsi(char final, string p)
    {
        // cursor visible: ?25h / ?25l
        if (p.StartsWith("?25", StringComparison.Ordinal) && (final == 'h' || final == 'l'))
        {
            FlushText();
            OnOp(new TerminalOp.SetCursorVisible(final == 'h'));
            return;
        }

        switch (final)
        {
            case 'm':
                FlushText();
                OnOp(new TerminalOp.SetSgr(p));
                break;

            case 'J':
                FlushText();
                OnOp(new TerminalOp.EraseDisplay((EraseDisplayMode)ParseFirstByte(p)));
                break;

            case 'K':
                FlushText();
                OnOp(new TerminalOp.EraseLine((EraseLineMode)ParseFirstByte(p)));
                break;

            case 'H':
            case 'f':
                FlushText();
                var (r, c) = ParseRowCol(p);
                OnOp(new TerminalOp.SetCursorPos(r, c));
                break;
        }
    }

    static byte ParseFirstByte(string p) => byte.TryParse(p.Split(';')[0], out var b) ? b : (byte)0;

    static (short r, short c) ParseRowCol(string p)
    {
        var parts = p.Split(';');
        short Parse(int idx, short d) => idx < parts.Length && short.TryParse(parts[idx], out var v) && v > 0 ? v : d;
        return (Parse(0, 1), Parse(1, 1));
    }

    void FlushText()
    {
        if (_text.Length == 0) return;
        OnOp(new TerminalOp.AppendText(_text.ToString()));
        _text.Clear();
    }

    const int EXTENDED_STARTUPINFO_PRESENT = 0x00080000;
    const int PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE = 0x00020016;

    [StructLayout(LayoutKind.Sequential)] struct COORD { public short X, Y; }
    [StructLayout(LayoutKind.Sequential)] struct SECURITY_ATTRIBUTES { public int nLength; public IntPtr lpSecurityDescriptor; public bool bInheritHandle; }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct STARTUPINFO
    {
        public int cb;
        public string lpReserved, lpDesktop, lpTitle;
        public int dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
        public short wShowWindow, cbReserved2;
        public IntPtr lpReserved2, hStdInput, hStdOutput, hStdError;
    }

    [StructLayout(LayoutKind.Sequential)] struct STARTUPINFOEX { public STARTUPINFO StartupInfo; public IntPtr lpAttributeList; }

    [StructLayout(LayoutKind.Sequential)]
    struct PROCESS_INFORMATION
    {
        public IntPtr hProcess, hThread;
        public int dwProcessId, dwThreadId;
    }

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool CreatePipe(out SafeFileHandle hReadPipe, out SafeFileHandle hWritePipe, ref SECURITY_ATTRIBUTES lpPipeAttributes, int nSize);

    [DllImport("kernel32.dll")]
    static extern int CreatePseudoConsole(COORD size, SafeFileHandle hInput, SafeFileHandle hOutput, int dwFlags, out IntPtr phPC);

    [DllImport("kernel32.dll")]
    static extern int ResizePseudoConsole(IntPtr hPC, COORD size);

    [DllImport("kernel32.dll")] static extern void ClosePseudoConsole(IntPtr hPC);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool CloseHandle(IntPtr hObject);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool InitializeProcThreadAttributeList(IntPtr lpAttributeList, int dwAttributeCount, int dwFlags, ref IntPtr lpSize);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool UpdateProcThreadAttribute(IntPtr lpAttributeList, int dwFlags, IntPtr Attribute, IntPtr lpValue, IntPtr cbSize, IntPtr lpPreviousValue, IntPtr lpReturnSize);

    [DllImport("kernel32.dll")] static extern void DeleteProcThreadAttributeList(IntPtr lpAttributeList);

    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    static extern bool CreateProcessW(
        string lpApplicationName,
        string lpCommandLine,
        IntPtr lpProcessAttributes,
        IntPtr lpThreadAttributes,
        bool bInheritHandles,
        int dwCreationFlags,
        IntPtr lpEnvironment,
        string lpCurrentDirectory,
        ref STARTUPINFOEX lpStartupInfo,
        out PROCESS_INFORMATION lpProcessInformation);
}
