using System.Windows;
using System.Windows.Controls;

namespace CvBuilder.Ui;
/// <summary>
/// Interaction logic for OkCancelDialog.xaml
/// </summary>
public partial class OkCancelDialog : Window
{
    public OkCancelDialog(string title, string okButtonText, UserControl content)
    {
        InitializeComponent();
        Title = title;
        OkButton.Content = okButtonText;
        ContentPresenter.Content = content;
    }

    private void Cancel_Button_Click(object sender, RoutedEventArgs e)
    {
        DialogResult = false;
    }

    private void OkButton_Click(object sender, RoutedEventArgs e)
    {
        DialogResult = true;
    }
}
