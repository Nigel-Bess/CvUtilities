using Fulfil.Visualization.ErrorLogging.ViewModels;
using System.Windows;
using System.Windows.Media;

namespace Fulfil.Visualization.ErrorLogging;

public static class UserInfo
{
    private static readonly SolidColorBrush ErrorColor = Brushes.Red;
    private static readonly SolidColorBrush InfoColor = Brushes.Black;
    private static readonly SolidColorBrush WarningColor = Brushes.Orange;
    private static readonly SolidColorBrush SuccessColor = Brushes.ForestGreen;
    public static LogViewModel Log { get; } = new();
    public static void LogInfo(string info) => LogMessage(info, InfoColor);

    public static void LogError(string error) => LogMessage(error, ErrorColor);
    public static void LogWarning(string warning) => LogMessage(warning, WarningColor);
    public static void LogSuccess(string message) => LogMessage(message, SuccessColor);

    private static void LogMessage(string message, SolidColorBrush color)
    {
        Application.Current.Dispatcher?.Invoke(() =>
        {
            var messageViewModel = new MessageViewModel
            {
                Text = message,
                Time = DateTime.Now.ToString("G"),
                Color = color
            };
            Log.Messages.Add(messageViewModel);
        });
    }
}
