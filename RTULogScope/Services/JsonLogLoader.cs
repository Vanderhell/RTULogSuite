using Newtonsoft.Json.Linq;
using System;
using System.Data;
using System.IO;
using System.Windows;

namespace RTULogScope
{
    /// <summary>
    /// Provides functionality to load newline-delimited JSON (NDJSON) logs
    /// into a DataTable for visualization or analysis.
    /// Each line in the file is expected to be a valid JSON object with a
    /// "timestamp" field and a "values" array containing key-value-unit triples.
    /// </summary>
    public static class JsonLogLoader
    {
        /// <summary>
        /// Loads a JSON log file into a DataTable.
        /// Assumes each line is a separate JSON object with a timestamp and an array of measurement values.
        /// Automatically creates columns based on the keys from the first line.
        /// Null or missing values are replaced with 0.0.
        /// </summary>
        /// <param name="path">Full path to the log file</param>
        /// <returns>Populated DataTable, or empty if loading fails</returns>
        public static DataTable Load(string path)
        {
            var table = new DataTable();

            try
            {
                // Read all lines (each line is a JSON object)
                var lines = File.ReadAllLines(path);
                if (lines.Length == 0) return table;

                // Use the first line to define DataTable columns
                var firstObj = JObject.Parse(lines[0]);
                table.Columns.Add("timestamp", typeof(string));

                var values = (JArray)firstObj["values"];
                foreach (JObject valueObj in values)
                {
                    string key = (string)valueObj["key"];
                    table.Columns.Add(key, typeof(double));
                }

                // Parse each line and populate DataTable
                foreach (var line in lines)
                {
                    if (string.IsNullOrWhiteSpace(line)) continue;

                    var obj = JObject.Parse(line);
                    var row = table.NewRow();
                    row["timestamp"] = (string)obj["timestamp"];

                    foreach (JObject valueObj in (JArray)obj["values"])
                    {
                        string key = (string)valueObj["key"];
                        JToken token = valueObj["value"];

                        // Replace nulls with 0.0 to prevent exception
                        double value = (token != null && token.Type != JTokenType.Null)
                            ? token.Value<double>()
                            : 0.0;

                        row[key] = value;
                    }

                    table.Rows.Add(row);
                }
            }
            catch (Exception ex)
            {
                // Show error dialog if JSON loading fails
                MessageBox.Show(
                    $"Error while reading JSON file:\n{path}\n\nDetails:\n{ex.Message}",
                    "JSON Read Error",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }

            return table;
        }
    }
}
