#include "StorageManager.h"
#include <ArduinoJson.h>

/// <summary>
/// Initializes the SD card interface.
/// Must be called once during system startup before using SD operations.
/// </summary>
void StorageManager::begin() {
    Serial.println("[StorageManager] Initializing SD card...");
    if (SD.begin()) {
        Serial.println("[StorageManager] SD card initialized successfully.");
    } else {
        Serial.println("[StorageManager][ERROR] Failed to initialize SD card.");
    }
}

/// <summary>
/// Checks whether the SD card is available and properly initialized.
/// This method reinitializes the SD interface to verify presence.
/// </summary>
/// <returns>True if SD card is available, false otherwise</returns>
bool StorageManager::isCardPresent() {
    bool available = SD.begin();  // Triggers a re-check
    Serial.printf("[StorageManager] SD card presence: %s\n", available ? "yes" : "no");
    return available;
}

/// <summary>
/// Generates today's log filename based on the current date,
/// using the configured folder and filename format (strftime style).
/// </summary>
/// <returns>Full path to the generated log file</returns>
String StorageManager::getTodayLogFilename() {
    time_t t = time(nullptr);
    struct tm* tm_info = localtime(&t);

    char buffer[64];
    strftime(buffer, sizeof(buffer), filenameFormat.c_str(), tm_info);

    String fullPath = outputFolder + String(buffer);
    Serial.printf("[StorageManager] Log filename generated: %s\n", fullPath.c_str());
    return fullPath;
}

/// <summary>
/// Writes a single log entry to the SD card in JSON format.
/// Each entry contains a timestamp and an array of key/value/unit objects.
/// Skips logging if SD is unavailable or configuration mismatch occurs.
/// </summary>
/// <param name="timestamp">Formatted timestamp string</param>
/// <param name="values">Vector of float values from Modbus</param>
/// <param name="registers">Vector of RegisterConfig defining keys and units</param>
void StorageManager::writeJSON(const String& timestamp, const std::vector<float>& values, const std::vector<RegisterConfig>& registers) {
    if (!loggingEnabled) {
        Serial.println("[StorageManager][WARN] Logging disabled.");
        return;
    }

    if (values.size() != registers.size()) {
        Serial.printf("[StorageManager][ERROR] Register/value count mismatch: %d vs %d\n", registers.size(), values.size());
        logError("Logging skipped due to register/value size mismatch.");
        return;
    }

    String filename = getTodayLogFilename();
    File file = SD.open(filename, FILE_APPEND);
    if (!file) {
        Serial.printf("[StorageManager][ERROR] Failed to open log file: %s\n", filename.c_str());
        logError("Failed to open log file: " + filename);
        return;
    }

    Serial.printf("[StorageManager] Writing log entry to %s\n", filename.c_str());

    // Create JSON structure
    DynamicJsonDocument doc(2048);
    doc["timestamp"] = timestamp;

    JsonArray valArray = doc.createNestedArray("values");
    for (size_t i = 0; i < values.size(); ++i) {
        JsonObject entry = valArray.createNestedObject();
        entry["key"] = registers[i].key;
        entry["value"] = values[i];
        entry["unit"] = registers[i].unit;
    }

    // Write to file
    serializeJson(doc, file);
    file.println();  // Ensure newline for NDJSON
    file.close();

    Serial.println("[StorageManager] Log entry saved.");
}

/// <summary>
/// Configures internal storage behavior:
/// target folder, filename pattern, and logging flags.
/// </summary>
/// <param name="folder">Directory to store log files</param>
/// <param name="format">Filename format (strftime-style)</param>
/// <param name="enable">Enable or disable logging</param>
/// <param name="withHeader">Whether to include CSV headers (reserved)</param>
void StorageManager::configure(const String& folder, const String& format, bool enable, bool withHeader) {
    outputFolder = folder;
    filenameFormat = format;
    loggingEnabled = enable;
    includeHeader = withHeader;

    Serial.println("[StorageManager] Logging configuration updated:");
    Serial.printf("  - Output folder: %s\n", outputFolder.c_str());
    Serial.printf("  - Filename format: %s\n", filenameFormat.c_str());
    Serial.printf("  - Logging enabled: %s\n", loggingEnabled ? "true" : "false");
    Serial.printf("  - Include header: %s\n", includeHeader ? "true" : "false");
}

/// <summary>
/// Logs an error message to a persistent error log file on the SD card,
/// prefixed with a timestamp for later diagnostics.
/// </summary>
/// <param name="message">Error message to log</param>
void StorageManager::logError(const String& message) {
    File errorFile = SD.open(errorLogFile, FILE_APPEND);
    if (errorFile) {
        time_t now = time(nullptr);
        errorFile.print("[");
        errorFile.print(ctime(&now));  // Includes newline
        errorFile.print("] ERROR: ");
        errorFile.println(message);
        errorFile.close();
    }
    Serial.printf("[StorageManager][LOG_ERROR] %s\n", message.c_str());
}
