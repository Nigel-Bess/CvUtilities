using System.ComponentModel;
using System.Reflection;
using System.Windows;

public static class Extension
{
    public static void InvokeIfRequired(this object control, Action action)
    {
        Application.Current.Dispatcher.Invoke(action);
    }

    public static string? GetDescription(this Enum value) =>
    value.GetType().GetField(value.ToString())?.GetCustomAttribute<DescriptionAttribute>()?.Description;
}