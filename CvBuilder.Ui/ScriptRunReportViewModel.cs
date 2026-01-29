using CvBuilder.Ui.Scripts;
using CvBuilder.Ui.Wpf;
using System.Windows;

namespace CvBuilder.Ui;

public class ScriptRunReportViewModel : Notifier
{
    public string ScriptName { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public double Progress { get => field; set { field = value; NotifyPropertyChanged(); } } = 0;
    public Visibility ProgressVisibility { get => field; set { field = value; NotifyPropertyChanged(); } } = Visibility.Collapsed;
    public Visibility CompleteVisibility { get => field; set { field = value; NotifyPropertyChanged(); } } = Visibility.Collapsed;
    public string CompleteText { get => field; set { field = value; NotifyPropertyChanged(); } } = "";
    public void Start(IScript script)
    {
        CompleteVisibility = Visibility.Collapsed;
        ProgressVisibility = Visibility.Visible;
        ScriptName = script.Name;
    }

    public void Clear()
    {
        CompleteVisibility = Visibility.Collapsed;
        ProgressVisibility = Visibility.Collapsed;
    }

    public void End(ScriptCompletionInfo completion)
    {
        CompleteVisibility = Visibility.Visible;
        ProgressVisibility = Visibility.Collapsed;
        CompleteText = completion.Succeeded ? "Done" : $"Failed: {completion.FailureReason}";
    }
}
