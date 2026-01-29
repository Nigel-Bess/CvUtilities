using CvBuilder.Ui.Wpf;
using System.Windows.Input;

namespace CvBuilder.Ui;

public class LoginFormViewModel
{
    public string Username { get; set; }
    public string Password { get; set; }
    public ICommand LoginCommand { get; }
    public ICommand CancelCommand { get; }
    public Action<bool> Close { get; set; }
    public LoginFormViewModel()
    {
        LoginCommand = new Command(() => Complete(true), () => !IsMissingInfo());
        CancelCommand = new Command(() => Complete(false));
    }
    private bool IsMissingInfo() => string.IsNullOrWhiteSpace(Username) || string.IsNullOrWhiteSpace(Password);
    public void Complete(bool result) => Close?.Invoke(result);
}
