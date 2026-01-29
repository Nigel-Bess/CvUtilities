using CvBuilder.Ui.Deploy;
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
                SaveDeployableBuilds([]);
                return [];
            }
            var uniqueBuilds = builds.ToHashSet();
            if (uniqueBuilds.Count != builds.Count)
            {
                builds = uniqueBuilds.ToList();

            }
            SaveDeployableBuilds(builds);
            return builds;
        }
        catch (Exception e)
        {
            UserInfo.LogError($"Unable to deserialize {nameof(settings.CompletedDispenseBuildsJson)}: {e.Message}: {jsonText}");
            SaveDeployableBuilds([]);
            return [];
        }
    }

    public static void SaveDeployableBuild(DeployableBuild build)
    {
        var builds = GetDeployableBuilds();
        builds.Add(build);
        builds = builds.Take(100).ToList();
        UserSettings.Default.CompletedDispenseBuildsJson = JsonHelpers.SerializeAsJson(builds.ToList());
        UserSettings.Default.Save();
    }

    private static void SaveDeployableBuilds(IEnumerable<DeployableBuild> builds)
    {
        UserSettings.Default.CompletedDispenseBuildsJson = JsonHelpers.SerializeAsJson(builds.Take(100).ToList());
        UserSettings.Default.Save();
    }
}
