using System.Diagnostics;

namespace CvBuilder;

public static class RunCmd
{
    public static async Task<(int ExitCode, string StdOut, string StdErr)> ExecuteAsync(string cmd, string directory)
    {
        using var p = new Process
        {
            StartInfo = new ProcessStartInfo("cmd.exe", $"/C {cmd}")
            {
                UseShellExecute = false,
                CreateNoWindow = true,
                WorkingDirectory = directory,
                RedirectStandardOutput = true,
                RedirectStandardError = true
            }
        };

        p.Start();

        var stdoutTask = p.StandardOutput.ReadToEndAsync();
        var stderrTask = p.StandardError.ReadToEndAsync();

        await Task.WhenAll(stdoutTask, stderrTask, p.WaitForExitAsync());
        return (p.ExitCode, await stdoutTask, await stderrTask);
    }
}
