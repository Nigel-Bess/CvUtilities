using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
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


    /// <summary>
    /// Attempts to find the only item in a sequence.
    /// </summary>
    /// <returns>false if sequence contains no elements or sequence contains more than one element</returns>
    public static bool TryGetSingle<T>(this IEnumerable<T> enumerable, [NotNullWhen(returnValue: true)] out T? single) where T : notnull => enumerable.TryGetSingle(t => true, out single);
    /// <summary>
    /// Attempts to find the only item in a sequence matching the given condition.
    /// </summary>
    /// <returns>true if an one and only one item was found that matches the given condition</returns>
    public static bool TryGetSingle<T>(this IEnumerable<T> enumerable, Predicate<T> condition, [NotNullWhen(returnValue: true)] out T? single) where T : notnull
    {
        var found = false;
        single = default(T?);
        foreach (var item in enumerable.Where(i => condition(i)))
        {
            if (found)
            { // already found one, and now we found another? -> fail
                single = default(T?);
                return false;
            }
            single = item;
            found = true;
        }
        return found;
    }

    /// <summary>
    /// Attempts to find the first item in a sequence.
    /// </summary>
    /// <returns>false if sequence contains no elements</returns>
    public static bool TryGetFirst<T>(this IEnumerable<T> enumerable, [NotNullWhen(returnValue: true)] out T? first) where T : notnull => enumerable.TryGetFirst(t => true, out first);

    /// <summary>
    /// Attempts to find the first item in a sequence matching the given condition
    /// </summary>
    /// <returns>true if an item was found that matches the given condition</returns>
    public static bool TryGetFirst<T>(this IEnumerable<T> enumerable, Predicate<T> condition, [NotNullWhen(returnValue: true)] out T? first) where T : notnull
    {
        foreach (var item in enumerable)
        {
            if (condition(item))
            {
                first = item;
                return true;
            }
        }
        first = default(T?);
        return false;
    }

}