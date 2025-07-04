// EyeIconConverter.cs
// RTULogScope – Converts boolean to an eye/hidden icon for use in ToggleButtons.

using System;
using System.Globalization;
using System.Windows.Data;

namespace RTULogScope.ViewModels.Converters
{
    /// <summary>
    /// Converts a boolean value to an emoji icon representing visibility:
    /// - true  → 👁️ (visible)
    /// - false → 🙈 (hidden)
    /// Used in ToggleButton content to visually indicate state.
    /// </summary>
    public class EyeIconConverter : IValueConverter
    {
        /// <summary>
        /// Converts a boolean value to a Unicode emoji icon.
        /// </summary>
        /// <param name="value">Boolean to convert</param>
        /// <returns>👁️ if true, 🙈 if false</returns>
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool isVisible = value is bool b && b;
            return isVisible ? "👁️" : "🙈";
        }

        /// <summary>
        /// Backward conversion is not used.
        /// </summary>
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return Binding.DoNothing;
        }
    }
}
