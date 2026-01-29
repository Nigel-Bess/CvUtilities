using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Wpf;
using Fulfil.Visualization.ErrorLogging;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;

namespace CvBuilder.Ui.Deploy;

public class BuildAndDeployViewModel
{
    public ScriptRunner ScriptRunner { get; }
    public ICommand StartBuildCommand { get; }
    public ObservableCollection<DeployViewModel> DeployableBuilds { get; } = new();

    public string BuildBranchText
    {
        get => UserSettings.Default.DispenseBuildBranch;
        set
        {
            UserSettings.Default.DispenseBuildBranch = value;
        }
    }


    public Facility SelectedFacility
    {
        get => (Facility)(UserSettings.Default.DispenseBuildFacility);
        set
        {
            UserSettings.Default.DispenseBuildFacility = (int)value;
        }
    }
    public IEnumerable<Facility> FacilityOptions { get; }
    public BuildAndDeployViewModel(ScriptRunner scriptRunner)
    {
        ScriptRunner = scriptRunner;
        StartBuildCommand = new Command(StartBuild, CanStartBuild);
        FacilityOptions = Enum.GetValues<Facility>().Where(e => e != Facility.None).ToList();
    }

    private bool CanStartBuild()
    {
        if (!ScriptRunner.IsIdle()) return false;
        if (string.IsNullOrWhiteSpace(BuildBranchText)) return false;
        if (SelectedFacility == Facility.None) return false;
        return true;
    }


    public async void StartBuild()
    {
        UserSettings.Default.Save();
        var facilityName = SelectedFacility.GetDescription();
        if (facilityName is null)
        {
            UserInfo.LogError($"No {nameof(DescriptionAttribute)} defined for {SelectedFacility}");
            return;
        }
        var branchName = BuildBranchText;
        var build = new BuildDispense(branchName: branchName, facilityName: facilityName);
        var result = await ScriptRunner.Run(build);
        if (!result.Succeeded)
        {
            UserInfo.LogError(result.FailureReason);
            return;
        }
        DeployableBuilds.Add(new(branchName: branchName, facilityName: facilityName, ScriptRunner));
    }
}
