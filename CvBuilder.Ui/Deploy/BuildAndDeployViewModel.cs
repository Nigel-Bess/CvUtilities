using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Wpf;
using System.Windows.Input;

namespace CvBuilder.Ui.Deploy;

public class BuildAndDeployViewModel
{
    public ScriptRunner ScriptRunner { get; }
    public ICommand StartBuildCommand { get; }
    public string BuildBranchText { get; set; } = "";
    public string BuildFacilityText { get; set; } = "";
    public BuildAndDeployViewModel(ScriptRunner scriptRunner)
    {
        ScriptRunner = scriptRunner;
        StartBuildCommand = new Command(StartBuild, CanStartBuild);
    }

    private bool CanStartBuild()
    {
        if (!ScriptRunner.IsIdle()) return false;
        if (string.IsNullOrWhiteSpace(BuildBranchText)) return false;
        if (string.IsNullOrWhiteSpace(BuildFacilityText)) return false;
        return true;
    }


    public async void StartBuild()
    {
        var build = new BuildDispense(branchName: BuildBranchText, facilityName: BuildFacilityText);
        await ScriptRunner.Run(build);
    }
}
