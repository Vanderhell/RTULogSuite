#include "DataLogger.h"

/// <summary>
/// Constructs a new instance of DataLogger.
/// </summary>
/// <param name="rtc">Pointer to RTC manager for obtaining timestamps</param>
/// <param name="storage">Pointer to StorageManager for writing JSON logs</param>
/// <param name="modbus">Pointer to ModbusManager for register reads</param>
/// <param name="config">Pointer to ConfigManager to access register definitions</param>
DataLogger::DataLogger(RtcManager* rtc, StorageManager* storage, ModbusManager* modbus, ConfigManager* config)
    : rtc(rtc), storage(storage), modbus(modbus), config(config) {
    Serial.println("[DataLogger] Instance created.");
}

/// <summary>
/// Performs a complete data logging cycle:
/// - Retrieves current time
/// - Reads all configured Modbus registers
/// - Writes the values to SD card via StorageManager
/// Logs an error if any step fails or if count mismatch occurs.
/// </summary>
void DataLogger::logAll() {
    Serial.println("[DataLogger] Logging cycle started...");

    // Step 1: Get current timestamp
    String timestamp = rtc->getFormattedTime();
    Serial.printf("[DataLogger] Timestamp: %s\n", timestamp.c_str());

    // Step 2: Load register definitions from config
    std::vector<RegisterConfig> registers = config->getRegisters();
    Serial.printf("[DataLogger] Loaded %d register(s) for logging.\n", registers.size());

    // Step 3: Read values from Modbus
    std::vector<float> values = modbus->readAll(registers);
    Serial.printf("[DataLogger] Retrieved %d value(s) from Modbus.\n", values.size());

    // Step 4: Validate and write results
    if (!values.empty() && values.size() == registers.size()) {
        Serial.println("[DataLogger] Writing values to storage...");
        storage->writeJSON(timestamp, values, registers);
        Serial.println("[DataLogger] Logging completed successfully.");
    } else {
        Serial.println("[DataLogger][ERROR] Modbus read failed or mismatch in register/value count.");
        storage->logError("Modbus read failed or register/value count mismatch.");
    }
}
