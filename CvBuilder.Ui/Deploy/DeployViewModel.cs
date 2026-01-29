using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Util;
using CvBuilder.Ui.Wpf;
using Fulfil.Visualization.ErrorLogging;
using System.Windows.Input;

namespace CvBuilder.Ui.Deploy;

public class DeployViewModel
{
    public string BuildName => _build.ToString();
    public Facility Facility => _build.Facility;
    public ICommand DeployCommand { get; }
    public ICommand RemoveCommand { get; }
    private readonly ScriptRunner _runner;
    private readonly DeployableBuild _build;
    public Action<DeployViewModel> OnRemove { get; set; }
    public DeployViewModel(DeployableBuild build, ScriptRunner runner)
    {
        _runner = runner;
        _build = build;
        DeployCommand = new Command(Deploy, runner.IsIdle);
        RemoveCommand = new Command(Remove);
    }

    public async void Deploy()
    {
        var deployScript = new DeployDispense(_build);
        var result = await _runner.Run(deployScript);
        if (!result.Succeeded) UserInfo.LogError(result.FailureReason!);
    }

    public void Remove()
    {
        SettingsHelpers.RemoveDeployableBuild(_build);
        OnRemove?.Invoke(this);
    }
}
