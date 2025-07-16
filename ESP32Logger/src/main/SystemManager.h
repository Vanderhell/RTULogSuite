#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "ConfigManager.h"
#include "RtcManager.h"
#include "StorageManager.h"
#include "ModbusManager.h"
#include "DataLogger.h"

/// <summary>
/// Central system controller for managing hardware initialization,
/// configuration loading, data logging, and Modbus communication.
/// Orchestrates full system startup and periodic operation.
/// </summary>
class SystemManager {
public:
    /// <summary>
    /// Performs full system initialization including RTC, SD card,
    /// configuration loading, Modbus setup, and logger instantiation.
    /// Should be called once inside the `setup()` function.
    /// </summary>
    void setupAll();

    /// <summary>
    /// Returns the polling interval for data logging,
    /// loaded from the configuration file.
    /// </summary>
    /// <returns>Polling interval in milliseconds</returns>
    unsigned long getPollingInterval() const;

    /// <summary>
    /// Executes a single data acquisition and logging cycle.
    /// Should be called periodically inside the `loop()` function.
    /// </summary>
    void runCycle();

    /// <summary>
    /// Accessor for configuration manager (for debugging or testing).
    /// </summary>
    ConfigManager* getConfig() { return &config; }

    /// <summary>
    /// Accessor for RTC manager (for debugging or testing).
    /// </summary>
    RtcManager* getRtc() { return &rtc; }

    /// <summary>
    /// Allows manual setup of RTC time via serial terminal.
    /// Useful when RTC is not properly initialized or has invalid time.
    /// </summary>
    void setupRTCFromSerial();

private:
    RtcManager rtc;
    ConfigManager config;
    StorageManager storage;
    ModbusManager modbus;
    DataLogger logger;
};

#endif // SYSTEM_MANAGER_H
