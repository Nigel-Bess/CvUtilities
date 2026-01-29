using System.Windows;
using System.Windows.Controls;

namespace CvBuilder.Ui.Terminal
{
    /// <summary>
    /// Interaction logic for TerminalView.xaml
    /// </summary>
    public partial class TerminalView : UserControl
    {
        public TerminalView()
        {
            InitializeComponent();
            Loaded += OnLoaded;
            DataContextChanged += OnDataContextChanged;
            ScrollToBottom();
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            if (DataContext is not TerminalViewModel vm) return;
            AttachToVm(vm);
        }

        private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (e.OldValue is TerminalViewModel oldVm)
            {
                DetachFromVm(oldVm);
            }

            // attach to new VM
            if (e.NewValue is TerminalViewModel newVm)
            {
                AttachToVm(newVm);
            }
        }

        private void DetachFromVm(TerminalViewModel vm)
        {
            vm.OnGotText -= OnMessagesChanged;
        }

        private void AttachToVm(TerminalViewModel vm)
        {
            vm.OnGotText += OnMessagesChanged;
        }

        private void OnMessagesChanged()
        {
            // Check if we were at the bottom before new items
            var atBottom = ScrollViewer.VerticalOffset + ScrollViewer.ViewportHeight
                           >= ScrollViewer.ExtentHeight - 1;
            if (!atBottom) return;
            ScrollToBottom();

        }


        private void ScrollToBottom() => this.InvokeIfRequired(ScrollViewer.ScrollToEnd);
    }
}
