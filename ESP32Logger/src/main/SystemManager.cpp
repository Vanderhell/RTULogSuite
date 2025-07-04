#include "SystemManager.h"
#include "RtcManager.h"
#include "ConfigManager.h"
#include "StorageManager.h"
#include "ModbusManager.h"
#include "DataLogger.h"

// Global instances of core components
RtcManager rtc;
ConfigManager config;
StorageManager storage;
ModbusManager modbus;
DataLogger logger(&rtc, &storage, &modbus, &config);

// Perform full system initialization.
// Called once during boot.
// Initializes RTC, SD card, loads config, and sets up Modbus.
void SystemManager::setupAll() {
    rtc.begin();

    // Prompt for RTC time if not yet set
    if (rtc.getFormattedTime().startsWith("1970") || rtc.getFormattedTime().startsWith("2000")) {
        rtc.serialSetupTime();  // only run if RTC time is invalid
    }

    storage.begin();
    config.setStorage(&storage);
    config.load();
    modbus.begin(config.getModbusSettings());
}

// Execute a single data acquisition and logging cycle.
// Checks for SD card and logs data if available.
// Should be called periodically in the main loop.
void SystemManager::runCycle() {
    if (storage.isCardPresent()) {
        logger.logAll();
    } else {
        storage.logError("SD card not present.");
    }
}
