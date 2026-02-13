
using System.Windows;

namespace CvBuilder.Ui.Util;

public static class Extension
{
    public static void InvokeIfRequired(this object control, Action action)
    {
        Application.Current.Dispatcher.Invoke(action);
    }
}
