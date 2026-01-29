using System.Windows.Media;

namespace Fulfil.Visualization.ErrorLogging.ViewModels;

public class MessageViewModel
{
    public required string Text { get; init; }
    public required string Time { get; init; }
    public required SolidColorBrush Color { get; init; }
}
