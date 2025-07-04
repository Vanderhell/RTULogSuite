#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <RTClib.h>

// Manages access to the DS3231 Real-Time Clock (RTC).
// Provides basic initialization, time formatting, and manual
// time-setting functionality for accurate timestamping.
class RtcManager {
public:
    // Initialize the RTC hardware.
    void begin();

    // Get the current RTC time formatted as a string.
    // Format: "YYYY-MM-DD HH:MM:SS"
    String getFormattedTime();

    // Manually set the RTC time and date.
    void setTime(uint16_t year, uint8_t month, uint8_t day,
                 uint8_t hour, uint8_t minute, uint8_t second);

    // Interactively set RTC time via Serial if time is invalid.
    // Prompts the user to enter date and time in format:
    // YYYY-MM-DD HH:MM:SS and sets RTC accordingly.
    void serialSetupTime();

private:
    RTC_DS3231 rtc; // RTClib instance for DS3231 communication
};

#endif // RTC_MANAGER_H
