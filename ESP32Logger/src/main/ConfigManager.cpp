#include "ConfigManager.h"

// Set the storage backend to use for logging and configuration persistence.
void ConfigManager::setStorage(StorageManager* s) {
    storage = s;
}

// Returns whether debug mode is enabled.
bool ConfigManager::isDebugEnabled() {
    return debugEnabled;
}

// Load configuration from the SD card.
// This function opens a JSON file, parses it,
// and initializes Modbus settings, polling interval, and register list.
// Also sets up logging if a storage manager is assigned.
void ConfigManager::load() {
    File file = SD.open("/config/config.json", FILE_READ);
    if (!file) {
        if (storage) storage->logError("Config load failed: cannot open config.json");
        return;
    }

    const size_t capacity = 8192;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, file);
    debugEnabled = doc["debug"] | false;

    file.close();

    if (error) {
        String msg = "Config load failed: invalid JSON format - ";
        msg += error.c_str();
        if (storage) storage->logError(msg);
        return;
    }

    pollingInterval = doc["logging"]["interval_ms"] | 1000;

    JsonObject comm = doc["communication"];
    modbusSettings.slave_id = comm["modbus_id"] | 1;
    modbusSettings.baudrate = comm["baudrate"] | 9600;
    String parityStr = comm["parity"] | "N";
    modbusSettings.parity = parityStr.charAt(0);
    modbusSettings.stop_bits = comm["stop_bits"] | 1;
    modbusSettings.data_bits = comm["data_bits"] | 8;

    registers.clear();
    JsonArray regs = doc["registers"];
    for (JsonObject reg : regs) {
        RegisterConfig r;
        r.key = reg["key"].as<String>();
        r.name = reg["name"].as<String>();
        r.description = reg["description"].as<String>();
        r.register_address = reg["register"] | 0;
        r.type = reg["type"].as<String>();
        r.unit = reg["unit"].as<String>();
        r.scaling = reg["scaling"].as<String>();
        r.access = reg["access"].as<String>();
        r.length = reg["length"] | 1;
        registers.push_back(r);
    }

    if (storage) {
        JsonObject log = doc["logging"];
        String folder = log["output_folder"] | "/";
        String format = log["filename_format"] | "data_%Y%m%d.csv";
        bool enabled = log["enabled"] | true;
        bool withHeader = log["include_header"] | true;
        storage->configure(folder, format, enabled, withHeader);
    }

    Serial.println("Configuration loaded.");
}

// Get the configured polling interval in milliseconds.
unsigned long ConfigManager::getPollingInterval() {
    return pollingInterval;
}

// Get the configured Modbus communication parameters.
ModbusSettings ConfigManager::getModbusSettings() {
    return modbusSettings;
}

// Get the list of configured Modbus registers.
std::vector<RegisterConfig> ConfigManager::getRegisters() {
    return registers;
}
