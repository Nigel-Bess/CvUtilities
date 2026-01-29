using System.Diagnostics;
using System.Runtime.InteropServices;

namespace CvBuilder;

public sealed class CmdHost : IDisposable
{
    readonly Process _p;

    public CmdHost() => _p = Start();

    Process Start()
    {
        var p = new Process
        {
            StartInfo = new ProcessStartInfo
            {
                FileName = "cmd.exe",
                Arguments = "/K",
                UseShellExecute = false,
                CreateNoWindow = false,
                WindowStyle = ProcessWindowStyle.Normal
            }
        };
        p.Start();
        return p;
    }
    public async Task SendToConsoleAsync(string text)
    {
    }
    public void Execute(string cmd) => SendToConsole(cmd + "\r\n");

    void SendToConsole(string text)
    {
        if (_p.HasExited) throw new InvalidOperationException("cmd.exe has exited.");
        if (!AttachConsole((uint)_p.Id)) throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());

        try
        {
            var h = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, IntPtr.Zero, OPEN_EXISTING, 0, IntPtr.Zero);
            if (h == INVALID_HANDLE_VALUE) throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());

            try
            {
                var records = BuildKeyEvents(text);
                if (!WriteConsoleInputW(h, records, (uint)records.Length, out _))
                    throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
            }
            finally { CloseHandle(h); }
        }
        finally { FreeConsole(); }
    }



    static INPUT_RECORD[] BuildKeyEvents(string s)
    {
        var r = new INPUT_RECORD[s.Length * 2];
        for (var i = 0; i < s.Length; i++)
        {
            var c = s[i];
            r[i * 2 + 0] = INPUT_RECORD.Key(true, c);
            r[i * 2 + 1] = INPUT_RECORD.Key(false, c);
        }
        return r;
    }

    public void Dispose()
    {
        try { if (!_p.HasExited) Execute("exit"); } catch { }
        try { if (!_p.HasExited) _p.Kill(entireProcessTree: true); } catch { }
        _p.Dispose();
    }

    const ushort KEY_EVENT = 0x0001;
    const uint GENERIC_READ = 0x80000000, GENERIC_WRITE = 0x40000000;
    const uint FILE_SHARE_READ = 0x00000001, FILE_SHARE_WRITE = 0x00000002;
    const uint OPEN_EXISTING = 3;
    static readonly IntPtr INVALID_HANDLE_VALUE = new(-1);

    [StructLayout(LayoutKind.Explicit)]
    struct INPUT_RECORD
    {
        [FieldOffset(0)] public ushort EventType;
        [FieldOffset(4)] public KEY_EVENT_RECORD KeyEvent;

        public static INPUT_RECORD Key(bool down, char ch) => new()
        {
            EventType = KEY_EVENT,
            KeyEvent = new KEY_EVENT_RECORD
            {
                bKeyDown = down,
                wRepeatCount = 1,
                wVirtualKeyCode = 0,
                wVirtualScanCode = 0,
                UnicodeChar = ch,
                dwControlKeyState = 0
            }
        };
    }

    [StructLayout(LayoutKind.Sequential)]
    struct KEY_EVENT_RECORD
    {
        [MarshalAs(UnmanagedType.Bool)] public bool bKeyDown;
        public ushort wRepeatCount, wVirtualKeyCode, wVirtualScanCode;
        public char UnicodeChar;
        public uint dwControlKeyState;
    }

    [DllImport("kernel32.dll", SetLastError = true)] static extern bool AttachConsole(uint dwProcessId);
    [DllImport("kernel32.dll", SetLastError = true)] static extern bool FreeConsole();

    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    static extern IntPtr CreateFile(string name, uint access, uint share, IntPtr sa, uint creation, uint flags, IntPtr template);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool WriteConsoleInputW(IntPtr h, INPUT_RECORD[] buffer, uint length, out uint written);

    [DllImport("kernel32.dll", SetLastError = true)] static extern bool CloseHandle(IntPtr h);
}
