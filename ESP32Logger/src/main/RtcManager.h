#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <RTClib.h>

/// <summary>
/// Manages access to the DS3231 Real-Time Clock (RTC).
/// Provides basic initialization, time retrieval and formatting,
/// as well as interactive manual time setting via Serial.
/// </summary>
class RtcManager {
public:
    /// <summary>
    /// Initializes the DS3231 RTC hardware.
    /// Should be called once during system startup.
    /// </summary>
    void begin();

    /// <summary>
    /// Retrieves the current RTC time and returns it as a formatted string.
    /// Format: "YYYY-MM-DD HH:MM:SS"
    /// </summary>
    /// <returns>Formatted date-time string from RTC</returns>
    String getFormattedTime();

    /// <summary>
    /// Sets the RTC time and date manually using individual components.
    /// </summary>
    /// <param name="year">4-digit year (e.g., 2025)</param>
    /// <param name="month">Month (1–12)</param>
    /// <param name="day">Day of the month (1–31)</param>
    /// <param name="hour">Hour (0–23)</param>
    /// <param name="minute">Minute (0–59)</param>
    /// <param name="second">Second (0–59)</param>
    void setTime(uint16_t year, uint8_t month, uint8_t day,
                 uint8_t hour, uint8_t minute, uint8_t second);

    /// <summary>
    /// Prompts the user via Serial to manually enter the current time.
    /// If RTC time is invalid, allows entry in format: "YYYY-MM-DD HH:MM:SS".
    /// Automatically parses and applies the entered value.
    /// </summary>
    void serialSetupTime();

private:
    RTC_DS3231 rtc; ///< RTClib instance for DS3231 communication
};

#endif // RTC_MANAGER_H
