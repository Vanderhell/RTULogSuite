#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <SD.h>
#include <vector>
#include "RegisterConfig.h"

/// <summary>
/// Manages SD card logging operations, including:
/// - Writing log entries in JSON format
/// - Error logging to persistent file
/// - File and folder naming based on date
/// </summary>
class StorageManager {
public:
    /// <summary>
    /// Initializes the SD card interface.
    /// Must be called once during system startup before performing any file operations.
    /// </summary>
    void begin();

    /// <summary>
    /// Checks whether the SD card is available and initialized.
    /// This reinitializes the SD interface for verification.
    /// </summary>
    /// <returns>True if the card is present and accessible, false otherwise</returns>
    bool isCardPresent();

    /// <summary>
    /// Writes a single log entry in JSON format to the SD card.
    /// Each entry includes a timestamp and an array of measurement objects (key/value/unit).
    /// </summary>
    /// <param name="timestamp">Timestamp string (e.g., "2025-07-03 12:00:00")</param>
    /// <param name="values">Float values corresponding to registers</param>
    /// <param name="registers">Register definitions with key and unit</param>
    void writeJSON(const String& timestamp, const std::vector<float>& values, const std::vector<RegisterConfig>& registers);

    /// <summary>
    /// Appends an error message with timestamp to a persistent error log file on the SD card.
    /// </summary>
    /// <param name="message">Error message to be logged</param>
    void logError(const String& message);

    /// <summary>
    /// Configures logging behavior, including output folder and filename format.
    /// </summary>
    /// <param name="folder">Target output folder (e.g., "/logs/")</param>
    /// <param name="format">Filename format (e.g., "log_%Y%m%d.json") using strftime syntax</param>
    /// <param name="enable">True to enable logging, false to disable</param>
    /// <param name="withHeader">Whether to include CSV headers (not used in JSON)</param>
    void configure(const String& folder, const String& format, bool enable, bool withHeader);

private:
    /// <summary>
    /// Generates the full path for today's log file using the current date and configured format.
    /// </summary>
    /// <returns>Full SD card file path for today's log</returns>
    String getTodayLogFilename();

    String errorLogFile = "error.log";                 // Error log filename
    String outputFolder = "/";                         // Output directory
    String filenameFormat = "data_%Y%m%d.json";        // Filename format (strftime-compatible)
    bool loggingEnabled = true;                        // Enable/disable logging
    bool includeHeader = true;                         // Reserved for future CSV support
};

#endif // STORAGE_MANAGER_H
