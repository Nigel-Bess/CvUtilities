using System.IO;

namespace CvBuilder.Ui.Util;

public class PathHelpers
{
    /// <summary>
    /// Generates full path %temp%/<paramref name="fileNameWithExtension"/>
    /// </summary>
    public static string GenerateTempFilePath(string fileNameWithExtension) => Path.Combine(Path.GetTempPath(), fileNameWithExtension);
}