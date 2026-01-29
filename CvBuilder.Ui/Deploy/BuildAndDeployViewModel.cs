using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Wpf;
using System.Windows.Input;

namespace CvBuilder.Ui.Deploy;

public class BuildAndDeployViewModel
{
    public ScriptRunner ScriptRunner { get; }
    public ICommand StartBuildCommand { get; }
    public string BuildTargetText { get; set; }
    public BuildAndDeployViewModel(ScriptRunner scriptRunner)
    {
        ScriptRunner = scriptRunner;
        StartBuildCommand = new Command(StartBuild, ScriptRunner.IsIdle);
    }


    public void StartBuild()
    {
        var ssh = new Ssh(SshLogin.WhismanDab);
        ScriptRunner.Run(ssh);
    }
}
