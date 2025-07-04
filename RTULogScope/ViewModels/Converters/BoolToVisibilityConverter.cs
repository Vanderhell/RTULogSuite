// BoolToVisibilityConverter.cs
// RTULogScope – Converts a boolean value to Visibility (Visible/Collapsed) for UI binding.

using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace RTULogScope.ViewModels.Converters
{
    /// <summary>
    /// Converts a boolean to a Visibility value.
    /// True  → Visibility.Visible  
    /// False → Visibility.Collapsed
    /// </summary>
    public class BoolToVisibilityConverter : IValueConverter
    {
        /// <summary>
        /// Converts a boolean value to Visibility.
        /// </summary>
        /// <param name="value">Boolean value (true/false)</param>
        /// <returns>Visible if true, otherwise Collapsed</returns>
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return (value is bool b && b) ? Visibility.Visible : Visibility.Collapsed;
        }

        /// <summary>
        /// Converts Visibility back to boolean.
        /// </summary>
        /// <param name="value">Visibility value</param>
        /// <returns>True if Visible, otherwise false</returns>
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is Visibility v && v == Visibility.Visible;
        }
    }
}


//Visibility="{Binding IsChecked, ElementName=ToggleStats, Converter={StaticResource BoolToVisibilityConverter}}"