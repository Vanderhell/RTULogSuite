// MainWindow.xaml.cs
// RTULogScope – Main application window logic for CSV data loading and graph exporting.

using Microsoft.Win32;
using OxyPlot;
using OxyPlot.Wpf;
using RTULogScope.ViewModels;
using System;
using System.IO;
using System.Linq;
using System.Windows;

namespace RTULogScope.Views
{
    public partial class MainWindow : Window
    {
        /// <summary>
        /// Strongly typed access to the DataContext (GraphViewModel)
        /// </summary>
        private GraphViewModel ViewModel => (GraphViewModel)DataContext;

        public MainWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Handles the CSV file loading button click.
        /// Allows user to select one or more CSV files and loads them into the application.
        /// </summary>
        private void LoadCsv_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog
            {
                Filter = "CSV files (*.csv)|*.csv",
                InitialDirectory = AppDomain.CurrentDomain.BaseDirectory,
                Multiselect = true,
                Title = "Select one or more CSV files"
            };

            if (dialog.ShowDialog() == true && dialog.FileNames.Length > 0)
            {
                try
                {
                    ViewModel.LoadMultipleCsvs(dialog.FileNames);

                    // Set window title based on first loaded file
                    string fileName = Path.GetFileName(dialog.FileNames[0]);
                    this.Title = $"RTULogScope – {fileName}";
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Error loading CSV files:\n{ex.Message}",
                        "File Load Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        /// <summary>
        /// Handles the PNG export button click.
        /// Exports the currently displayed chart to a PNG image file.
        /// </summary>
        private void ExportToPng_Click(object sender, RoutedEventArgs e)
        {
            if (ViewModel.PlotModel == null || ViewModel.PlotModel.Series.Count == 0)
            {
                MessageBox.Show("Nothing to export – no data series selected.",
                                "Export Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            var dialog = new SaveFileDialog
            {
                Filter = "PNG image (*.png)|*.png",
                FileName = $"chart_{DateTime.Now:yyyyMMdd_HHmmss}.png",
                Title = "Save Chart as PNG"
            };

            if (dialog.ShowDialog() == true)
            {
                try
                {
                    var exporter = new PngExporter
                    {
                        Width = 1200,
                        Height = 800,
                    };

                    exporter.ExportToFile(ViewModel.PlotModel, dialog.FileName);

                    MessageBox.Show("Chart has been successfully exported.",
                                    "Export Complete", MessageBoxButton.OK, MessageBoxImage.Information);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Export failed:\n{ex.Message}",
                                    "Export Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }
    }
}
