// ContainsConverter.cs
// RTULogScope – MultiValueConverter to check if a collection contains a given item.
// Used for dynamic checkbox states in measurement selection.

using System;
using System.Collections;
using System.Globalization;
using System.Windows.Data;

namespace RTULogScope.ViewModels.Converters
{
    /// <summary>
    /// Checks whether a given collection contains a specific item.
    /// Typically used in WPF MultiBinding with checkbox lists.
    /// </summary>
    public class ContainsConverter : IMultiValueConverter
    {
        /// <summary>
        /// Returns true if the first value (IList) contains the second value (item).
        /// </summary>
        /// <param name="values">[0] = IList, [1] = item to check</param>
        /// <returns>True if item is in list; false otherwise</returns>
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            var list = values[0] as IList;
            var item = values[1];

            return list != null && item != null && list.Contains(item);
        }

        /// <summary>
        /// ConvertBack is not implemented – this converter is read-only.
        /// </summary>
        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException("ConvertBack is not supported in this context.");
        }
    }
}
