using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Util;
using CvBuilder.Ui.Wpf;
using Fulfil.Visualization.ErrorLogging;
using System.Collections.ObjectModel;
using System.Windows.Input;

namespace CvBuilder.Ui.DeployDispense;

public class BuildAndDeployViewModel : Notifier
{
    public ScriptRunner ScriptRunner { get; }
    public ICommand StartBuildCommand { get; }
    public ICommand DeployWithoutBuildingCommand { get; }
    public ObservableCollection<DeployViewModel> DeployableBuilds { get; }
    public double Progress { get => field; set { field = value; NotifyPropertyChanged(); } } = 0;
    public bool ShowProgress { get => field; set { field = value; NotifyPropertyChanged(); } } = false;
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
    public Dispense? SelectedBuildBox
    {
        get
        {
            var name = UserSettings.Default.SelectedBuildBoxName;
            return BuildBoxOptions.FirstOrDefault(b => b.Name == name);
        }
        set
        {
            UserSettings.Default.SelectedBuildBoxName = value?.Name ?? "";
        }
    }
    public IEnumerable<Dispense> BuildBoxOptions { get; }
    public BuildAndDeployViewModel(ScriptRunner scriptRunner)
    {
        ScriptRunner = scriptRunner;
        StartBuildCommand = new Command(StartBuild, CanStartBuild);
        DeployWithoutBuildingCommand = new Command(DeployWithoutBuilding, ScriptRunner.IsIdle);
        FacilityOptions = Enum.GetValues<Facility>().Where(e => e != Facility.None).ToList();
        BuildBoxOptions = BuildMachine.All();
        DeployableBuilds = new(SettingsHelpers.GetDeployableBuilds().Select(NewDeployViewModel));
    }

    private bool CanStartBuild()
    {
        if (SelectedBuildBox is null) return false;
        if (!ScriptRunner.IsIdle()) return false;
        if (string.IsNullOrWhiteSpace(BuildBranchText)) return false;
        if (SelectedFacility == Facility.None) return false;
        return true;
    }

    public void DeployWithoutBuilding()
    {
        var build = new DeployableBuild(BranchName: BuildBranchText, Facility: SelectedFacility);
        var vm = new DeployViewModel(build, ScriptRunner);
        vm.Deploy();
    }


    public async void StartBuild()
    {
        UserSettings.Default.Save();
        var branchName = BuildBranchText.Trim();
        var build = new BuildDispenseScript(branchName: branchName, facility: SelectedFacility, buildBox: SelectedBuildBox);
        var result = await ScriptRunner.Run(build);
        if (!result.Succeeded)
        {
            UserInfo.LogError(result.FailureReason);
            return;
        }
        var buildOutput = build.Output;
        SettingsHelpers.SaveDeployableBuild(buildOutput);
        DeployableBuilds.Add(NewDeployViewModel(buildOutput));
    }

    private DeployViewModel NewDeployViewModel(DeployableBuild build) => new(build, ScriptRunner) { OnRemove = OnRemoveBuild };

    private void OnRemoveBuild(DeployViewModel model) => DeployableBuilds.Remove(model);
}

