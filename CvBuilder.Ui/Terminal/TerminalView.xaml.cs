using CvBuilder.Ui.Util;
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
            vm.OutputReset -= OnOutputReset;
            vm.OutputAppended -= OnOutputAppended;
        }


        private void AttachToVm(TerminalViewModel vm)
        {
            vm.OutputReset += OnOutputReset;
            vm.OutputAppended += OnOutputAppended;
        }

        private void OnOutputReset()
        {
            this.InvokeIfRequired(() =>
            {
                OutputTextBox.Clear();
                ScrollToBottom();
            });
        }

        private void OnOutputAppended(string s)
        {
            this.InvokeIfRequired(() =>
            {
                // Check if we were at the bottom before appending
                var atBottom = ScrollViewer.VerticalOffset + ScrollViewer.ViewportHeight
                               >= ScrollViewer.ExtentHeight - 1;

                OutputTextBox.AppendText(s);
                if (atBottom) ScrollToBottom();
            });
        }

        private void ScrollToBottom() => this.InvokeIfRequired(ScrollViewer.ScrollToEnd);
    }
}
