#include "DataLogger.h"

// Construct a new DataLogger instance.
// Parameters:
// - rtc: pointer to RtcManager for timestamping
// - storage: pointer to StorageManager for writing logs
// - modbus: pointer to ModbusManager to read data
// - config: pointer to ConfigManager for access to register definitions
DataLogger::DataLogger(RtcManager* rtc, StorageManager* storage, ModbusManager* modbus, ConfigManager* config)
    : rtc(rtc), storage(storage), modbus(modbus), config(config) {}

// Log a complete snapshot of all configured Modbus registers.
// Steps:
// 1. Get current timestamp
// 2. Fetch register definitions from ConfigManager
// 3. Read all values from Modbus
// 4. Write timestamped values to storage
// If counts mismatch or reading fails, log an error.
void DataLogger::logAll() {
    // Get current timestamp as a formatted string
    String timestamp = rtc->getFormattedTime();

    // Retrieve register configuration
    std::vector<RegisterConfig> registers = config->getRegisters();

    // Read all configured registers via Modbus
    std::vector<float> values = modbus->readAll(registers);

    // If the number of read values matches the config, write them to storage
    if (!values.empty() && values.size() == registers.size()) {
        storage->writeJSON(timestamp, values, registers);
    } else {
        storage->logError("Modbus read failed or register/value count mismatch.");
    }
}
