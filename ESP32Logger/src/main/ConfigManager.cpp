#include "ConfigManager.h"

/// <summary>
/// Returns the configured polling interval in milliseconds.
/// </summary>
unsigned long ConfigManager::getPollingInterval() const {
    return pollingInterval;
}

/// <summary>
/// Returns the configured voltage transformer ratio.
/// </summary>
float ConfigManager::getVTR() { return transformerVTR; }

/// <summary>
/// Returns the configured current transformer ratio.
/// </summary>
float ConfigManager::getCTR() { return transformerCTR; }

/// <summary>
/// Returns the Modbus register address for reading VTR from device.
/// </summary>
uint16_t ConfigManager::getVTRRegister() const { return vtrRegister; }

/// <summary>
/// Returns the Modbus register address for reading CTR from device.
/// </summary>
uint16_t ConfigManager::getCTRRegister() const { return ctrRegister; }

/// <summary>
/// Attaches a StorageManager instance for error logging and configuration.
/// </summary>
void ConfigManager::setStorage(StorageManager* s) {
    storage = s;
    Serial.println("[ConfigManager] Storage manager attached.");
}

/// <summary>
/// Returns true if debug mode is enabled in configuration.
/// </summary>
bool ConfigManager::isDebugEnabled() {
    return debugEnabled;
}

/// <summary>
/// Loads configuration from "/config/config.json" on the SD card.
/// Initializes communication settings, polling interval, transformer ratios,
/// Modbus register definitions, and logging configuration.
/// </summary>
void ConfigManager::load() {
    Serial.println("[ConfigManager] Attempting to open /config/config.json...");

    File file = SD.open("/config/config.json", FILE_READ);
    if (!file) {
        Serial.println("[ConfigManager][ERROR] Failed to open config.json!");
        if (storage) storage->logError("Config load failed: cannot open config.json");
        return;
    }

    Serial.println("[ConfigManager] config.json opened successfully.");

    const size_t capacity = 8192;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, file);
    file.close(); // Important: close file after deserialization

    if (error) {
        Serial.println("[ConfigManager][ERROR] JSON deserialization failed:");
        Serial.println(error.c_str());
        if (storage) {
            String msg = "Config load failed: invalid JSON format - ";
            msg += error.c_str();
            storage->logError(msg);
        }
        return;
    }

    Serial.println("[ConfigManager] JSON deserialized successfully.");

    // Debug flag
    debugEnabled = doc["debug"] | false;
    if (debugEnabled) Serial.println("[ConfigManager] Debug mode enabled.");

    // Polling interval
    pollingInterval = doc["logging"]["interval_ms"] | 1000;
    Serial.printf("[ConfigManager] Polling interval set to %lu ms.\n", pollingInterval);

    // Modbus communication settings
    JsonObject comm = doc["communication"];
    modbusSettings.slave_id = comm["modbus_id"] | 1;
    modbusSettings.baudrate = comm["baudrate"] | 9600;
    String parityStr = comm["parity"] | "N";
    modbusSettings.parity = parityStr.charAt(0);
    modbusSettings.stop_bits = comm["stop_bits"] | 1;
    modbusSettings.data_bits = comm["data_bits"] | 8;

    // Addressing mode
    String addrMode = comm["addressing_mode"] | "0-based";
    addressOffsetEnabled = (addrMode == "1-based");

    Serial.printf("[ConfigManager] Modbus settings loaded:\n");
    Serial.printf("  - Slave ID: %d\n", modbusSettings.slave_id);
    Serial.printf("  - Baudrate: %ld\n", modbusSettings.baudrate);
    Serial.printf("  - Parity: %c\n", modbusSettings.parity);
    Serial.printf("  - Stop bits: %d\n", modbusSettings.stop_bits);
    Serial.printf("  - Data bits: %d\n", modbusSettings.data_bits);
    Serial.printf("[ConfigManager] Addressing mode: %s (offset %s)\n", addrMode.c_str(), addressOffsetEnabled ? "-1" : "0");

    // Transformer ratios and optional register mapping
    JsonObject trans = doc["transformers"];
    transformerVTR = trans["VTR"] | 1.0f;
    transformerCTR = trans["CTR"] | 1.0f;
    vtrRegister = trans["VTR_register"] | 0;
    ctrRegister = trans["CTR_register"] | 0;
    Serial.printf("[ConfigManager] Transformer ratios: VTR=%.2f, CTR=%.2f\n", transformerVTR, transformerCTR);
    Serial.printf("[ConfigManager] Transformer register addresses: VTR=%u, CTR=%u\n", vtrRegister, ctrRegister);

    // Register definitions
    if (!doc.containsKey("registers")) {
        Serial.println("[ConfigManager][ERROR] Missing 'registers' key in JSON.");
        return;
    }

    JsonArray regs = doc["registers"];
    Serial.printf("[ConfigManager] Found %d register(s).\n", regs.size());
    registers.clear();

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

        Serial.printf("  - [%s] %s @ %d (%s), scaling: %s\n",
                      r.key.c_str(), r.name.c_str(),
                      r.register_address, r.type.c_str(),
                      r.scaling.c_str());
    }

    // Logging configuration
    if (storage) {
        JsonObject log = doc["logging"];
        String folder = log["output_folder"] | "/";
        String format = log["filename_format"] | "data_%Y%m%d.csv";
        bool enabled = log["enabled"] | true;
        bool withHeader = log["include_header"] | true;

        Serial.println("[ConfigManager] Logging configuration:");
        Serial.printf("  - Folder: %s\n", folder.c_str());
        Serial.printf("  - Filename format: %s\n", format.c_str());
        Serial.printf("  - Enabled: %s\n", enabled ? "true" : "false");
        Serial.printf("  - Include header: %s\n", withHeader ? "true" : "false");

        storage->configure(folder, format, enabled, withHeader);
    }

    Serial.println("[ConfigManager] Configuration loaded successfully.");
}

/// <summary>
/// Returns the configured Modbus communication settings.
/// </summary>
ModbusSettings ConfigManager::getModbusSettings() {
    return modbusSettings;
}

/// <summary>
/// Returns the list of Modbus register configurations loaded from JSON.
/// </summary>
std::vector<RegisterConfig> ConfigManager::getRegisters() {
    return registers;
}
