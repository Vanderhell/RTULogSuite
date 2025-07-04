// CsvLoader.cs
// RTULogScope – Loads CSV files into a DataTable for visualization.
// Expects comma-separated values with a timestamp column and numeric measurements.

using System;
using System.Data;
using System.IO;
using System.Windows;

namespace RTULogScope.Services
{
    /// <summary>
    /// Provides a method to load simple CSV files into a DataTable.
    /// </summary>
    public static class CsvLoader
    {
        /// <summary>
        /// Loads a CSV file into a DataTable.
        /// Assumes the first row contains column headers and values are comma-separated.
        /// </summary>
        /// <param name="path">Full file path to the CSV</param>
        /// <returns>Populated DataTable, or empty if failed</returns>
        public static DataTable Load(string path)
        {
            var table = new DataTable();

            try
            {
                using (var reader = new StreamReader(path))
                {
                    var headers = reader.ReadLine()?.Split(',');
                    if (headers == null)
                        return table;

                    // Add columns from the first line
                    foreach (var h in headers)
                        table.Columns.Add(h.Trim());

                    // Read all data rows
                    while (!reader.EndOfStream)
                    {
                        var line = reader.ReadLine();
                        if (string.IsNullOrWhiteSpace(line)) continue;

                        var values = line.Split(',');

                        // Basic protection: match column count
                        if (values.Length == table.Columns.Count)
                            table.Rows.Add(values);
                    }
                }
            }
            catch (IOException ex)
            {
                MessageBox.Show(
                    $"The file could not be loaded:\n{path}\n\nDetails:\n{ex.Message}",
                    "CSV Read Error",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    $"Unexpected error while reading CSV file:\n{path}\n\nDetails:\n{ex.Message}",
                    "Unexpected Error",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }

            return table;
        }
    }
}
