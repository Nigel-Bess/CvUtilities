using System.Windows;

public static class Extension
{
    public static void InvokeIfRequired(this object control, Action action)
    {
        Application.Current.Dispatcher.Invoke(action);
    }
}