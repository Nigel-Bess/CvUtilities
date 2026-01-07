using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace CvBuilder.Ui.Wpf;

public class Notifier : INotifyPropertyChanged
{
    public event PropertyChangedEventHandler? PropertyChanged;

    protected virtual void NotifyPropertyChanged([CallerMemberName] string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}