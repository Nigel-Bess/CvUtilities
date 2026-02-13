using System.Diagnostics.CodeAnalysis;

namespace CvBuilder.Ui;

public static class Slack
{
    public static bool TryGetToken([NotNullWhen(returnValue: true)] out string? token)
    {
        token = UserSettings.Default.SlackToken;
        if (!string.IsNullOrEmpty(token)) return true;
        return TryPromptForToken(out token);
    }

    public static bool TryPromptForToken([NotNullWhen(returnValue: true)] out string? token)
    {
        var vm = new SingleTextFormViewModel() { Label = "Token: " };
        var form = new SingleTextFieldForm() { DataContext = vm };
        var window = new OkCancelDialog("Enter Slack Token", "Ok", form) { Width = 300, Height = 200 };
        if (window.ShowDialog() == false)
        {
            token = null;
            return false;
        }
        token = vm.Value;
        UserSettings.Default.SlackToken = token;
        UserSettings.Default.Save();
        return true;
    }
}
