using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Util;
using CvBuilder.Ui.Wpf;
using Fulfil.Visualization.ErrorLogging;
using System.Windows.Input;

namespace CvBuilder.Ui.DeployDispense;

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
        var dispenseOptions = Dispense.GetMachines(Facility);
        if (!dispenseOptions.Any())
        {
            UserInfo.LogError($"No dispenses defined for {Facility}");
            return;
        }
        var selectionFormVm = new SelectionFormViewModel<Dispense>(dispenseOptions);
        var selectionForm = new SelectionForm() { DataContext = selectionFormVm };
        var window = new OkCancelDialog("Dab Selection", "Deploy", selectionForm);
        if (window.ShowDialog() != true) return;
        var dabs = selectionFormVm.Selected;
        if (!dabs.Any())
        {
            UserInfo.LogError($"Cannot deploy {_build}: No dabs selected");
            return;
        }
        if (dabs.TryGetSingle(out var singleDab))
        {
            DeploySingle(singleDab);
            return;
        }
        DeployMultiple(dabs);

        //var deployScript = new DeployDispenseScript(_build);
        //var result = await _runner.Run(deployScript);
        //if (!result.Succeeded) UserInfo.LogError(result.FailureReason!);
    }

    private void DeploySingle(Dispense dispense)
    {
        var script = new MockScript($"Deploy to {dispense}");
        _ = _runner.Run(script);
    }

    private void DeployMultiple(IEnumerable<Dispense> dispenes)
    {
        UserInfo.LogError($"Multi-deploy not yet implemented");
    }

    public void Remove()
    {
        SettingsHelpers.RemoveDeployableBuild(_build);
        OnRemove?.Invoke(this);
    }
}
