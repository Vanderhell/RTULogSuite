// GraphViewModel.cs
// RTULogScope – ViewModel responsible for loading CSV data, managing selections, and rendering the OxyPlot chart.

using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Series;
using RTULogScope.Models;
using RTULogScope.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;

namespace RTULogScope.ViewModels
{
    public class GraphViewModel : INotifyPropertyChanged
    {
        // Chart model to bind to PlotView
        public PlotModel PlotModel { get; private set; } = new PlotModel
        {
            Title = "RTU Log Scope",
            Background = OxyColors.White
        };

        // Summary statistics shown below the graph
        private string _statsSummary;
        public string StatsSummary
        {
            get => _statsSummary;
            set
            {
                _statsSummary = value;
                OnPropertyChanged();
            }
        }

        // Observable list of available measurement columns
        public ObservableCollection<SelectableItem<string>> Measurements { get; set; } = new ObservableCollection<SelectableItem<string>>();

        // Table holding all loaded measurement data
        private DataTable data;

        /// <summary>
        /// Loads a single CSV file and populates the Measurements collection.
        /// </summary>
        public void LoadCsv(string path)
        {
            data = CsvLoader.Load(path);
            Measurements.Clear();

            if (data == null || !data.Columns.Contains("timestamp"))
            {
                Debug.WriteLine("❌ CSV does not contain 'timestamp' column or is empty.");
                return;
            }

            foreach (DataColumn col in data.Columns)
            {
                if (!string.Equals(col.ColumnName, "timestamp", StringComparison.OrdinalIgnoreCase))
                {
                    var item = new SelectableItem<string>(col.ColumnName);
                    item.SelectionChanged += (_, __) => UpdatePlot();
                    Measurements.Add(item);
                }
            }

            UpdatePlot();
        }

        /// <summary>
        /// Loads and merges multiple CSV files, sorts data by timestamp.
        /// </summary>
        public void LoadMultipleCsvs(string[] paths)
        {
            var merged = new DataTable();

            foreach (var path in paths)
            {
                var table = CsvLoader.Load(path);
                if (table.Rows.Count == 0) continue;

                if (merged.Columns.Count == 0)
                {
                    foreach (DataColumn col in table.Columns)
                        merged.Columns.Add(col.ColumnName);
                }

                foreach (DataRow row in table.Rows)
                    merged.ImportRow(row);
            }

            if (!merged.Columns.Contains("timestamp"))
            {
                Debug.WriteLine("❌ No CSV file contains a 'timestamp' column.");
                return;
            }

            var rows = merged.AsEnumerable()
                .Where(r => DateTime.TryParse(r["timestamp"]?.ToString(), out _))
                .OrderBy(r => DateTime.Parse(r["timestamp"].ToString()))
                .ToList();

            if (rows.Count == 0)
            {
                Debug.WriteLine("⚠️ No valid timestamp rows found.");
                return;
            }

            data = rows.CopyToDataTable(); // safe fallback

            Measurements.Clear();
            foreach (DataColumn col in data.Columns)
            {
                if (!string.Equals(col.ColumnName, "timestamp", StringComparison.OrdinalIgnoreCase))
                {
                    var item = new SelectableItem<string>(col.ColumnName);
                    item.SelectionChanged += (_, __) => UpdatePlot();
                    Measurements.Add(item);
                }
            }

            UpdatePlot();
        }

        /// <summary>
        /// Rebuilds the plot and statistics based on selected measurements.
        /// </summary>
        public void UpdatePlot()
        {
            if (data == null) return;

            var selected = Measurements.Where(m => m.IsSelected).ToList();

            PlotModel.Series.Clear();
            PlotModel.Axes.Clear();

            PlotModel.Axes.Add(new DateTimeAxis
            {
                Position = AxisPosition.Bottom,
                Title = "Time",
                StringFormat = "HH:mm:ss",
                IsZoomEnabled = true,
                IsPanEnabled = true
            });

            PlotModel.Axes.Add(new LinearAxis
            {
                Position = AxisPosition.Left,
                Title = "Value"
            });

            foreach (var item in selected)
            {
                if (!data.Columns.Contains(item.Value)) continue;

                var series = new LineSeries { Title = item.Value };
                int count = 0;

                foreach (DataRow row in data.Rows)
                {
                    var tsRaw = row["timestamp"]?.ToString()?.Trim();
                    var valRaw = row[item.Value]?.ToString()?.Trim();

                    if (DateTime.TryParse(tsRaw, out var time) &&
                        double.TryParse(valRaw?.Replace(',', '.'), System.Globalization.NumberStyles.Any,
                            System.Globalization.CultureInfo.InvariantCulture, out var value))
                    {
                        series.Points.Add(new DataPoint(DateTimeAxis.ToDouble(time), value));
                        count++;
                    }
                }

                PlotModel.Series.Add(series);
                // Debug.WriteLine($"✅ {item.Value}: {count} points added");
            }

            // Generate statistics
            var statLines = new List<string>();

            foreach (var item in selected)
            {
                if (!data.Columns.Contains(item.Value)) continue;

                var values = data.Rows.Cast<DataRow>()
                    .Select(r => r[item.Value]?.ToString()?.Trim())
                    .Select(s => double.TryParse(s?.Replace(',', '.'), System.Globalization.NumberStyles.Any,
                                 System.Globalization.CultureInfo.InvariantCulture, out var v) ? (double?)v : null)
                    .Where(v => v.HasValue)
                    .Select(v => v.Value)
                    .ToList();

                if (values.Count > 0)
                {
                    double min = values.Min();
                    double max = values.Max();
                    double avg = values.Average();

                    statLines.Add($"{item.Value}: Min = {min:F2} | Max = {max:F2} | Avg = {avg:F2}");
                }
            }

            StatsSummary = string.Join("\n", statLines);

            PlotModel.InvalidatePlot(true);
        }

        /// <summary>
        /// Raises the PropertyChanged event for data binding.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        private void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
    }
}
