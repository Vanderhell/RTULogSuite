#include "StorageManager.h"
#include <ArduinoJson.h>

// Initialize the SD card interface.
// Must be called once during system startup before using SD operations.
void StorageManager::begin() {
    SD.begin();
}

// Check whether the SD card is available and initialized.
bool StorageManager::isCardPresent() {
    return SD.begin(); 
}

// Generate the current day's log filename based on date format.
// Uses filenameFormat and combines it with outputFolder.
String StorageManager::getTodayLogFilename() {
    time_t t = time(nullptr);
    struct tm* tm_info = localtime(&t);

    char buffer[64];
    strftime(buffer, sizeof(buffer), filenameFormat.c_str(), tm_info);

    return outputFolder + String(buffer);
}

// Write measurement values to a JSON log file on the SD card.
// Each log entry is stored as a single JSON line.
// If value count doesn't match register count, logging is skipped.
void StorageManager::writeJSON(const String& timestamp, const std::vector<float>& values, const std::vector<RegisterConfig>& registers) {
    if (!loggingEnabled || values.size() != registers.size()) {
        logError("Logging skipped or register/value size mismatch.");
        return;
    }

    String filename = getTodayLogFilename();
    File file = SD.open(filename, FILE_APPEND);
    if (!file) {
        logError("Failed to open log file: " + filename);
        return;
    }

    // Create JSON document with timestamp and value array
    DynamicJsonDocument doc(2048);
    doc["timestamp"] = timestamp;

    JsonArray valArray = doc.createNestedArray("values");
    for (size_t i = 0; i < values.size(); ++i) {
        JsonObject entry = valArray.createNestedObject();
        entry["key"] = registers[i].key;
        entry["value"] = values[i];
        entry["unit"] = registers[i].unit;
    }

    serializeJson(doc, file);
    file.println();  // newline after each JSON object
    file.close();
}

// Configure the storage settings for logging.
// Called during config load to set folder, filename format, and flags.
void StorageManager::configure(const String& folder, const String& format, bool enable, bool withHeader) {
    outputFolder = folder;
    filenameFormat = format;
    loggingEnabled = enable;
    includeHeader = withHeader;
}

// Append an error message to a persistent error log on the SD card.
// The log includes a timestamp and the message.
void StorageManager::logError(const String& message) {
    File errorFile = SD.open(errorLogFile, FILE_APPEND);
    if (errorFile) {
        time_t now = time(nullptr);
        errorFile.print("[");
        errorFile.print(ctime(&now));  // includes newline
        errorFile.print("] ERROR: ");
        errorFile.println(message);
        errorFile.close();
    }
}
