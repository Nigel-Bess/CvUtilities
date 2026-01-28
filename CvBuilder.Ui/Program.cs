namespace CvBuilder.Ui;

public static class Program
{
    public static void Start()
    {
        var mainWindow = new MainWindow() { DataContext = new MainWindowViewModel() };
        mainWindow.Show();
    }
}
