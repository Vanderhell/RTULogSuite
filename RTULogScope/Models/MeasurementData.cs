// MeasurementData.cs
// RTULogScope – Represents a single data point (timestamped measurement) used for plotting or analysis.

using System;

namespace RTULogScope.Models
{
    /// <summary>
    /// Represents a single timestamped measurement value (e.g. from Modbus RTU data).
    /// Used for graph plotting, statistics, or data export.
    /// </summary>
    public class MeasurementData
    {
        /// <summary>
        /// The timestamp associated with this measurement.
        /// </summary>
        public DateTime Timestamp { get; set; }

        /// <summary>
        /// The name of the measured parameter (e.g. "voltage_l1-n").
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// The numeric value of the measurement.
        /// </summary>
        public double Value { get; set; }
    }
}
