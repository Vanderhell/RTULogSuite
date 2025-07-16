#include "RtcManager.h"

/// <summary>
/// Initializes the DS3231 RTC module.
/// Verifies connection, checks for power loss,
/// and syncs RTC time with the system time.
/// </summary>
void RtcManager::begin() {
    if (!rtc.begin()) {
        Serial.println("[RtcManager][ERROR] RTC module not found or not connected!");
    } else {
        Serial.println("[RtcManager] RTC module initialized.");
    }

    if (rtc.lostPower()) {
        Serial.println("[RtcManager][WARN] RTC lost power. Time may be invalid.");
    }

    // Get time from RTC and apply it to the ESP system time
    DateTime now = rtc.now();
    struct tm t;
    t.tm_year = now.year() - 1900;
    t.tm_mon  = now.month() - 1;
    t.tm_mday = now.day();
    t.tm_hour = now.hour();
    t.tm_min  = now.minute();
    t.tm_sec  = now.second();
    time_t timeNow = mktime(&t);
    struct timeval nowTime = { .tv_sec = timeNow, .tv_usec = 0 };
    settimeofday(&nowTime, nullptr);
}

/// <summary>
/// Returns the current date and time from RTC as a formatted string.
/// Format: "YYYY-MM-DD HH:MM:SS"
/// </summary>
String RtcManager::getFormattedTime() {
    DateTime now = rtc.now();
    char buf[20];

    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());

    return String(buf);
}

/// <summary>
/// Sets the RTC to the specified date and time.
/// </summary>
/// <param name="year">4-digit year</param>
/// <param name="month">Month (1–12)</param>
/// <param name="day">Day of month (1–31)</param>
/// <param name="hour">Hour (0–23)</param>
/// <param name="minute">Minute (0–59)</param>
/// <param name="second">Second (0–59)</param>
void RtcManager::setTime(uint16_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint8_t second) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.printf("[RtcManager] RTC set to: %04d-%02d-%02d %02d:%02d:%02d\n",
                  year, month, day, hour, minute, second);
}

/// <summary>
/// Allows the user to manually set RTC time via Serial input.
/// If no input is provided, sets RTC from current ESP system time.
/// Expected input format: "YYYY-MM-DD HH:MM:SS" or full command "setrtc ...".
/// </summary>
void RtcManager::serialSetupTime() {
    if (!Serial) Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println("📅 [RtcManager] Enter time to update RTC.");
    Serial.println("  ↪ You can enter:");
    Serial.println("     1. Nothing → sets from ESP system time");
    Serial.println("     2. Format: YYYY-MM-DD HH:MM:SS");
    Serial.println("     3. Full command: setrtc YYYY-MM-DD HH:MM:SS");

    Serial.print("⏳ Waiting input (10s timeout)... ");

    String input = "";
    unsigned long start = millis();
    while (millis() - start < 10000) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') goto parse_input;
            input += c;
            Serial.print(c);
        }
    }

parse_input:
    Serial.println();
    input.trim();

    // Support optional "setrtc" command prefix
    if (input.startsWith("setrtc")) {
        input = input.substring(6);
        input.trim();
    }

    // Manual time entry
    if (input.length() >= 19) {
        int y, M, d, h, m, s;
        if (sscanf(input.c_str(), "%d-%d-%d %d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
            setTime(y, M, d, h, m, s);
            Serial.printf("✅ [RtcManager] RTC manually set to %04d-%02d-%02d %02d:%02d:%02d\n", y, M, d, h, m, s);
            return;
        } else {
            Serial.println("❌ [RtcManager] Invalid format. Expected: YYYY-MM-DD HH:MM:SS");
        }
    }

    // Fallback: use current ESP system time
    time_t t = time(nullptr);
    struct tm* now = localtime(&t);
    if (now) {
        setTime(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                now->tm_hour, now->tm_min, now->tm_sec);
        Serial.printf("✅ [RtcManager] RTC set from system time: %04d-%02d-%02d %02d:%02d:%02d\n",
                      now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                      now->tm_hour, now->tm_min, now->tm_sec);
    } else {
        Serial.println("❌ [RtcManager] Failed to get system time.");
    }
}
