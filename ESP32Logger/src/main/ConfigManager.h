#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <SD.h>
#include "RegisterConfig.h"
#include "StorageManager.h"

class StorageManager;

// Structure for holding Modbus serial communication settings.
struct ModbusSettings {
    uint8_t slave_id;     // Modbus slave address
    long baudrate;        // Serial baud rate
    char parity;          // Parity ('N', 'E', or 'O')
    uint8_t stop_bits;    // Number of stop bits (usually 1)
    uint8_t data_bits;    // Number of data bits (usually 8)
};

// Loads and manages device configuration from a JSON file on SD card.
// Provides access to Modbus settings, polling intervals, and register definitions.
class ConfigManager {
public:
    // Attach a storage handler for error logging and configuration of output.
    void setStorage(StorageManager* storage);

    // Load configuration from SD card.
    // Reads and parses JSON file located at /config/config.json.
    void load();

    // Get the data polling interval in milliseconds.
    // Returns configured interval or default (1000 ms).
    unsigned long getPollingInterval();

    // Get the current Modbus communication settings.
    ModbusSettings getModbusSettings();

    // Retrieve the list of Modbus register configurations.
    std::vector<RegisterConfig> getRegisters();

    // Check if debug mode is enabled from config.
    bool isDebugEnabled();

private:
    StorageManager* storage = nullptr;     // Optional logging and file interface
    unsigned long pollingInterval;         // Polling interval for data collection
    ModbusSettings modbusSettings;         // Modbus serial config
    std::vector<RegisterConfig> registers; // List of Modbus registers to read
    bool debugEnabled = false;             // Debug flag
};

#endif // CONFIG_MANAGER_H
