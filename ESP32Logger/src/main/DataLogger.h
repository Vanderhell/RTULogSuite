#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "RtcManager.h"
#include "StorageManager.h"
#include "ModbusManager.h"
#include "ConfigManager.h"

/// <summary>
/// Handles periodic logging of Modbus register values to persistent storage.
/// This class coordinates:
/// - Timestamping via RTC
/// - Register reading via Modbus
/// - Saving results to SD via StorageManager
/// </summary>
class DataLogger {
public:
    /// <summary>
    /// Default constructor (used only for placeholder instances).
    /// Must be initialized later using assignment or full constructor.
    /// </summary>
    DataLogger() = default;

    /// <summary>
    /// Constructs a new DataLogger instance with all required dependencies.
    /// </summary>
    /// <param name="rtc">Pointer to RTC manager (for current timestamp)</param>
    /// <param name="storage">Pointer to storage manager (for file output)</param>
    /// <param name="modbus">Pointer to Modbus manager (for data reading)</param>
    /// <param name="config">Pointer to configuration manager (for register definitions)</param>
    DataLogger(RtcManager* rtc, StorageManager* storage, ModbusManager* modbus, ConfigManager* config);

    /// <summary>
    /// Executes a single logging operation.
    /// Steps:
    /// 1. Gets current timestamp from RTC
    /// 2. Reads all configured Modbus registers
    /// 3. Saves the data to storage as JSON
    /// Logs errors in case of failure.
    /// </summary>
    void logAll();

private:
    RtcManager* rtc;            ///< Reference to RTC manager (for timestamps)
    StorageManager* storage;   ///< Reference to storage backend (SD card writer)
    ModbusManager* modbus;     ///< Reference to Modbus handler
    ConfigManager* config;     ///< Reference to register configuration source
};

#endif // DATA_LOGGER_H
