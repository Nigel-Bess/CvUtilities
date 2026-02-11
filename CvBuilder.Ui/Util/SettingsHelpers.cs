using CvBuilder.Ui.DeployDispense;
using Fulfil.Visualization.ErrorLogging;

namespace CvBuilder.Ui.Util;

public static class SettingsHelpers
{
    public static List<DeployableBuild> GetDeployableBuilds()
    {
        var settings = UserSettings.Default;
        var jsonText = settings.CompletedDispenseBuildsJson;
        try
        {
            var builds = JsonHelpers.DeserializeFromJson<List<DeployableBuild>>(jsonText);
            if (builds is null)
            {
                // don't log a warning - this is nominal behavior when no builds are saved
                return SanitizeAndSaveDeployableBuilds([]);
            }

            return SanitizeAndSaveDeployableBuilds(builds);
        }
        catch (Exception e)
        {
            UserInfo.LogError($"Unable to deserialize {nameof(settings.CompletedDispenseBuildsJson)}: {e.Message}: {jsonText}");
            return SanitizeAndSaveDeployableBuilds([]);
        }
    }

    public static void SaveDeployableBuild(DeployableBuild build) =>
        SanitizeAndSaveDeployableBuilds(GetDeployableBuilds().Prepend(build).ToList());
    public static void RemoveDeployableBuild(DeployableBuild build) =>
    SanitizeAndSaveDeployableBuilds(GetDeployableBuilds().Where(b => b != build).ToList());

    private static List<DeployableBuild> SanitizeAndSaveDeployableBuilds(List<DeployableBuild> builds)
    {
        var uniqueBuilds = builds.ToHashSet();
        if (uniqueBuilds.Count != builds.Count)
        {
            builds = uniqueBuilds.ToList();

        }
        var sanitized = builds.Where(b => b.IsValid()).Take(100).ToList();
        UserSettings.Default.CompletedDispenseBuildsJson = JsonHelpers.SerializeAsJson(sanitized);
        UserSettings.Default.Save();
        return sanitized;
    }
}
