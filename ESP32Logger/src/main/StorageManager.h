#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <SD.h>
#include <vector>
#include "RegisterConfig.h"

// Handles SD card logging and error reporting.
// Responsible for writing logs in JSON format,
// generating filenames based on date,
// and logging errors to a persistent file.
class StorageManager {
public:
    // Initialize the SD card interface.
    // Must be called before using any storage functionality.
    void begin();

    // Check if the SD card is present and accessible.
    bool isCardPresent();

    // Write a log entry in JSON format.
    // Each entry includes a timestamp and an array of register values.
    // Parameters:
    // - timestamp: e.g., "2025-07-03 12:00:00"
    // - values: float values read from Modbus
    // - registers: matching RegisterConfig definitions
    void writeJSON(const String& timestamp, const std::vector<float>& values, const std::vector<RegisterConfig>& registers);

    // Log an error message to persistent log on SD card.
    void logError(const String& message);

    // Configure the logging output.
    // Parameters:
    // - folder: path on SD card
    // - format: filename format with strftime syntax
    // - enable: enable or disable logging
    // - withHeader: include CSV header (not used for JSON)
    void configure(const String& folder, const String& format, bool enable, bool withHeader);

private:
    // Generate today's filename from folder + format
    String getTodayLogFilename();

    String errorLogFile = "error.log";        // Error log file path
    String outputFolder = "/";                // Directory for logs
    String filenameFormat = "data_%Y%m%d.json"; // Date-based filename format
    bool loggingEnabled = true;               // Global logging flag
    bool includeHeader = true;                // Whether to write CSV headers (for future use)
};

#endif // STORAGE_MANAGER_H
