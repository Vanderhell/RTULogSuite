#ifndef REGISTER_CONFIG_H
#define REGISTER_CONFIG_H

#include <Arduino.h>

/// <summary>
/// Represents the configuration for a single Modbus register or measurement parameter.
/// This struct is typically loaded from a JSON configuration file and contains all
/// metadata required for reading, interpreting, and logging a Modbus register value.
/// </summary>
struct RegisterConfig {
    /// <summary>
    /// Unique key identifier (used in logs, data export, and internal lookup).
    /// Example: "voltage_l1-n"
    /// </summary>
    String key;

    /// <summary>
    /// Human-readable name shown in UI or CSV headers.
    /// Example: "Voltage L1-N"
    /// </summary>
    String name;

    /// <summary>
    /// Full technical or user-friendly description of the register.
    /// Useful for diagnostics or documentation.
    /// </summary>
    String description;

    /// <summary>
    /// Modbus register address (starting from 0 or 1 depending on addressing mode).
    /// </summary>
    uint16_t register_address;

    /// <summary>
    /// Type of data stored in the register.
    /// Common types: "UINT16", "UINT32", "FLOAT"
    /// </summary>
    String type;

    /// <summary>
    /// Unit of the measurement, e.g. "V", "A", "Hz", "kWh", or "-"
    /// </summary>
    String unit;

    /// <summary>
    /// Optional scaling expression, used to convert raw Modbus values.
    /// Examples: "val * 0.1", "CTR * val / VTR"
    /// </summary>
    String scaling;

    /// <summary>
    /// Access mode of the register: "R-only", "R/W", etc.
    /// </summary>
    String access;

    /// <summary>
    /// Number of 16-bit Modbus registers used by this value.
    /// For example: 1 for UINT16, 2 for FLOAT/UINT32.
    /// </summary>
    uint8_t length;
};

#endif // REGISTER_CONFIG_H
