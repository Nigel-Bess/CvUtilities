using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Wpf;
using Fulfil.Visualization.ErrorLogging;
using System.Windows.Input;

namespace CvBuilder.Ui.Deploy;

public class DeployViewModel
{
    public string BranchName { get; }
    public string FacilityName { get; }
    public ICommand DeployCommand { get; }
    private readonly ScriptRunner _runner;
    public DeployViewModel(string branchName, string facilityName, ScriptRunner runner)
    {
        BranchName = branchName;
        FacilityName = facilityName;
        _runner = runner;
        DeployCommand = new Command(Deploy, runner.IsIdle);
    }

    public async void Deploy()
    {
        var deployScript = new DeployDispense();
        var result = await _runner.Run(deployScript);
        if (!result.Succeeded) UserInfo.LogError(result.FailureReason!);
    }
}
