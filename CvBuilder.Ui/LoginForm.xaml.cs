using System.Windows;
using System.Windows.Controls;

namespace CvBuilder.Ui;
/// <summary>
/// Interaction logic for LoginForm.xaml
/// </summary>
public partial class LoginForm : UserControl
{
    public LoginForm()
    {
        InitializeComponent();
        DataContextChanged += OnDataContextChanged;
    }


    private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
    {
        if (e.OldValue is LoginFormViewModel oldVm) oldVm.Close -= OnRequestClose;
        if (e.NewValue is LoginFormViewModel newVm) newVm.Close += OnRequestClose;
    }


    void OnRequestClose(bool ok) => Window.GetWindow(this)!.DialogResult = ok;
}
