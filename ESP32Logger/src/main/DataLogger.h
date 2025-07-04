#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "RtcManager.h"
#include "StorageManager.h"
#include "ModbusManager.h"
#include "ConfigManager.h"

// Handles periodic logging of Modbus register values to storage.
// This class ties together the RTC, Modbus communication,
// and storage manager to collect and log measurements with timestamps.
class DataLogger {
public:
    // Construct a new DataLogger object.
    // Parameters:
    // - rtc: pointer to real-time clock manager for timestamps
    // - storage: pointer to storage manager for file output
    // - modbus: pointer to Modbus manager for data retrieval
    // - config: pointer to configuration manager for register mapping
    DataLogger(RtcManager* rtc, StorageManager* storage, ModbusManager* modbus, ConfigManager* config);

    // Log all configured Modbus registers with current timestamp.
    // Reads the current time, fetches all register values,
    // and stores them via the storage backend.
    // Logs an error if data reading fails.
    void logAll();

private:
    RtcManager* rtc;          // Used to fetch current timestamp
    StorageManager* storage; // Responsible for saving logs
    ModbusManager* modbus;   // Performs Modbus read operations
    ConfigManager* config;   // Provides register configurations
};

#endif // DATA_LOGGER_H
