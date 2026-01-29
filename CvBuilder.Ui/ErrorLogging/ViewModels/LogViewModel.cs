using System.Collections.ObjectModel;

namespace Fulfil.Visualization.ErrorLogging.ViewModels;

public class LogViewModel
{
    public ObservableCollection<MessageViewModel> Messages { get; } = new();
}
