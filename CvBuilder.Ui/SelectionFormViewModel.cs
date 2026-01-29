namespace CvBuilder.Ui;

public class SelectionFormViewModel<T>
{
    public List<SelectableObject<T>> Options { get; }
    public IEnumerable<T> Selected => Options.Where(o => o.IsSelected).Select(o => o.Item);
    public SelectionFormViewModel(IEnumerable<T> options)
    {
        Options = options.Select(o => new SelectableObject<T>(o)).ToList();

    }
}
