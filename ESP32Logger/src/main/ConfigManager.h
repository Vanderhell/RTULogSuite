#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <SD.h>
#include "RegisterConfig.h"
#include "StorageManager.h"

class StorageManager;

/// <summary>
/// Structure for holding Modbus RTU serial communication parameters.
/// These values are typically loaded from JSON configuration.
/// </summary>
struct ModbusSettings {
    uint8_t slave_id;     ///< Modbus slave address
    long baudrate;        ///< Baud rate (e.g., 9600, 19200)
    char parity;          ///< Parity ('N' = None, 'E' = Even, 'O' = Odd)
    uint8_t stop_bits;    ///< Stop bits (usually 1)
    uint8_t data_bits;    ///< Data bits (usually 8)
};

/// <summary>
/// Manages application configuration loaded from SD card (JSON).
/// Provides Modbus communication settings, polling interval,
/// register definitions, transformer ratios, and debug flags.
/// </summary>
class ConfigManager {
public:
    /// <summary>
    /// Assigns a reference to the storage manager for logging errors.
    /// </summary>
    void setStorage(StorageManager* storage);

    /// <summary>
    /// Returns voltage transformer ratio loaded from config.
    /// </summary>
    float getVTR();

    /// <summary>
    /// Returns current transformer ratio loaded from config.
    /// </summary>
    float getCTR();

    /// <summary>
    /// Returns the Modbus register address where VTR is stored.
    /// </summary>
    uint16_t getVTRRegister() const;

    /// <summary>
    /// Returns the Modbus register address where CTR is stored.
    /// </summary>
    uint16_t getCTRRegister() const;

    /// <summary>
    /// Loads the configuration from "/config/config.json" on the SD card.
    /// Parses all required fields and stores them internally.
    /// </summary>
    void load();

    /// <summary>
    /// Returns the polling interval (in milliseconds) for data acquisition.
    /// Defaults to 1000 ms if not configured.
    /// </summary>
    unsigned long getPollingInterval() const;

    /// <summary>
    /// Returns the current Modbus communication settings.
    /// </summary>
    ModbusSettings getModbusSettings();

    /// <summary>
    /// Returns a list of all configured Modbus registers to read.
    /// </summary>
    std::vector<RegisterConfig> getRegisters();

    /// <summary>
    /// Returns true if debug mode is enabled in configuration.
    /// </summary>
    bool isDebugEnabled();

    /// <summary>
    /// Returns true if 1-based Modbus addressing (offset) is enabled.
    /// </summary>
    bool isAddressOffsetEnabled() const { return addressOffsetEnabled; }

private:
    StorageManager* storage = nullptr;              ///< Reference to logger/storage handler
    unsigned long pollingInterval = 1000;           ///< Interval between Modbus reads
    ModbusSettings modbusSettings;                  ///< Modbus serial configuration
    std::vector<RegisterConfig> registers;          ///< Configured list of Modbus registers
    bool debugEnabled = false;                      ///< Enables verbose debugging if true
    float transformerVTR = 1.0f;                    ///< Voltage transformer ratio
    float transformerCTR = 1.0f;                    ///< Current transformer ratio
    uint16_t vtrRegister = 0;                       ///< Optional register to read VTR from device
    uint16_t ctrRegister = 0;                       ///< Optional register to read CTR from device
    bool addressOffsetEnabled = false;              ///< Enables 1-based addressing (Modbus)
};

#endif // CONFIG_MANAGER_H
