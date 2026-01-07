using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace CvBuilder.Ui.Wpf;

/// <summary>
/// A command that provides <see cref="ICommand"/> functionality, with the addition of
/// accepting a <see cref="Func{TResult}"/> returning a <see cref="Task"/> as the execute
/// action, and providing an <see cref="ExecutionTask"/> property that notifies changes when
/// <see cref="ExecuteAsync"/> is invoked and when the returned <see cref="Task"/> completes.
/// </summary>
public sealed partial class AsyncRelayCommand : ICommand
{

    /// <summary>
    /// The <see cref="Func{TResult}"/> to invoke when <see cref="Execute"/> is used.
    /// </summary>
    private readonly Func<Task> execute;

    /// <summary>
    /// The optional action to invoke when <see cref="CanExecute"/> is used.
    /// </summary>
    private readonly Func<bool>? canExecute;

    /// <inheritdoc/>
    public event EventHandler? CanExecuteChanged
    {
        add => CommandManager.RequerySuggested += value;
        remove => CommandManager.RequerySuggested -= value;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="AsyncRelayCommand"/> class.
    /// </summary>
    /// <param name="execute">The execution logic.</param>
    /// <exception cref="System.ArgumentNullException">Thrown if <paramref name="execute"/> is <see langword="null"/>.</exception>
    public AsyncRelayCommand(Func<Task> execute)
    {
        ArgumentNullException.ThrowIfNull(execute);

        this.execute = execute;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="AsyncRelayCommand"/> class.
    /// </summary>
    /// <param name="execute">The execution logic.</param>
    /// <param name="canExecute">The execution status logic.</param>
    /// <exception cref="ArgumentNullException">Thrown if <paramref name="execute"/> or <paramref name="canExecute"/> are <see langword="null"/>.</exception>
    public AsyncRelayCommand(Func<Task> execute, Func<bool> canExecute)
    {
        ArgumentNullException.ThrowIfNull(execute);
        ArgumentNullException.ThrowIfNull(canExecute);

        this.execute = execute;
        this.canExecute = canExecute;
    }

    private Task? executionTask;

    /// <inheritdoc/>
    public Task? ExecutionTask
    {
        get => this.executionTask;
        private set
        {
            if (ReferenceEquals(this.executionTask, value))
            {
                return;
            }

            this.executionTask = value;

            bool isAlreadyCompletedOrNull = value?.IsCompleted ?? true;

            // The branch is on a condition evaluated before raising the events above if
            // needed, to avoid race conditions with a task completing right after them.
            if (isAlreadyCompletedOrNull)
            {
                return;
            }
        }
    }

    /// <inheritdoc/>
    public bool IsRunning => ExecutionTask is { IsCompleted: false };

    /// <inheritdoc/>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public bool CanExecute(object? parameter)
    {
        bool canExecute = this.canExecute?.Invoke() != false;

        return canExecute && (ExecutionTask is not { IsCompleted: false });
    }

    /// <inheritdoc/>
    public void Execute(object? parameter)
    {
        Task executionTask = ExecuteAsync(parameter);
    }

    /// <inheritdoc/>
    public Task ExecuteAsync(object? parameter) => ExecutionTask = this.execute();
}