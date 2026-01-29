using System.Diagnostics;

namespace CvBuilder;

public static class RunCmd
{
    const string Marker = "__FINAL_PWD__";

    public static async Task<(int ExitCode, string StdOut, string StdErr, string FinalDirectory)> ExecuteAsync(string cmd, string directory)
    {
        var wrapped = $"{cmd} & echo {Marker} & cd";

        using var p = new Process
        {
            StartInfo = new ProcessStartInfo("cmd.exe", $"/D /Q /C \"{wrapped.Replace("\"", "\\\"")}\"")
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

        var stdoutAll = await stdoutTask;
        var lines = stdoutAll.Split(new[] { "\r\n", "\n" }, StringSplitOptions.None);

        var i = Array.FindLastIndex(lines, l => l.Trim() == Marker);
        var finalDir = (i >= 0 && i + 1 < lines.Length ? lines[i + 1].Trim() : directory);

        var stdoutVisible = i >= 0
            ? string.Join(Environment.NewLine, lines.Take(i)).TrimEnd()
            : stdoutAll;

        return (p.ExitCode, stdoutVisible, await stderrTask, finalDir);
    }
}

