// SelectableItem.cs
// RTULogScope – Represents a bindable item with a selectable (boolean) state.
// Used in the ViewModel to track which measurements are selected for graph display.

using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace RTULogScope.Models
{
    /// <summary>
    /// A generic wrapper class for any value that can be selected (e.g. for checkboxes).
    /// Implements INotifyPropertyChanged for WPF data binding.
    /// </summary>
    /// <typeparam name="T">The underlying value type</typeparam>
    public class SelectableItem<T> : INotifyPropertyChanged
    {
        /// <summary>
        /// Gets the value associated with this item.
        /// </summary>
        public T Value { get; }

        private bool _isSelected;

        /// <summary>
        /// Indicates whether this item is selected (checked).
        /// Raises PropertyChanged and SelectionChanged when modified.
        /// </summary>
        public bool IsSelected
        {
            get => _isSelected;
            set
            {
                if (_isSelected != value)
                {
                    _isSelected = value;
                    OnPropertyChanged();
                    SelectionChanged?.Invoke(this, EventArgs.Empty);
                }
            }
        }

        /// <summary>
        /// Creates a new SelectableItem with the specified value.
        /// </summary>
        /// <param name="value">The value to wrap</param>
        public SelectableItem(T value)
        {
            Value = value;
        }

        /// <summary>
        /// Raised when the IsSelected property changes (for UI binding).
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Raised whenever selection changes – used for triggering plot updates.
        /// </summary>
        public event EventHandler SelectionChanged;

        /// <summary>
        /// Raises the PropertyChanged event for WPF binding.
        /// </summary>
        /// <param name="name">The name of the changed property</param>
        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
    }
}
