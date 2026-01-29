namespace CvBuilder.Ui;

public class SelectableObject<T>
{
    public bool IsSelected { get; set; } = false;
    public string DisplayString { get; }
    public T Item { get; }
    public SelectableObject(T underlying, Func<T, string>? toString = null)
    {
        toString ??= t => t?.ToString() ?? "NULL";
        DisplayString = toString(underlying);
    }
}
