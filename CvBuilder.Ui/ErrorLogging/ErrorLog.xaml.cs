using CvBuilder.Ui.Util;
using Fulfil.Visualization.ErrorLogging.ViewModels;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;

namespace Fulfil.Visualization.ErrorLogging
{
    /// <summary>
    /// Interaction logic for ErrorLog.xaml
    /// </summary>
    public partial class ErrorLog : UserControl
    {
        public ErrorLog()
        {
            InitializeComponent();
            Loaded += OnLoaded;
            DataContextChanged += OnDataContextChanged;
            ScrollToBottom();
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            if (DataContext is not LogViewModel vm) return;
            AttachToVm(vm);
        }

        private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (e.OldValue is LogViewModel oldVm)
            {
                DetachFromVm(oldVm);
            }

            // attach to new VM
            if (e.NewValue is LogViewModel newVm)
            {
                AttachToVm(newVm);
            }
        }

        private void DetachFromVm(LogViewModel vm)
        {
            vm.Messages.CollectionChanged -= OnMessagesChanged;
        }

        private void AttachToVm(LogViewModel vm)
        {
            vm.Messages.CollectionChanged += OnMessagesChanged;
        }

        private void OnMessagesChanged(object? sender, NotifyCollectionChangedEventArgs e)
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
