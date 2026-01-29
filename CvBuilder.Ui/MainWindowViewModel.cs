using CvBuilder.Ui.Deploy;
using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Wpf;
using System.Windows.Input;

namespace CvBuilder.Ui;

public class MainWindowViewModel
{
    public ICommand OpenGithubCredentials { get; }
    public required BuildAndDeployViewModel BuildAndDeployVm { get; init; }
    public MainWindowViewModel()
    {
        OpenGithubCredentials = new Command(() => _ = GitLogin.PromptForCredentials());
    }
}
