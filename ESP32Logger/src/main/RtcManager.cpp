#include "RtcManager.h"

// Initialize the RTC module.
// This should be called during startup to initialize the RTC hardware.
void RtcManager::begin() {
    rtc.begin();
}

// Get the current date and time formatted as a string.
// Format: "YYYY-MM-DD HH:MM:SS"
String RtcManager::getFormattedTime() {
    DateTime now = rtc.now();
    char buf[20];

    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());

    return String(buf);
}

// Manually set the RTC time and date.
void RtcManager::setTime(uint16_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint8_t second) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
}

// Prompt the user over Serial to set the RTC time manually.
// If the RTC time is invalid (1970 or 2000), asks for user input
// in format: YYYY-MM-DD HH:MM:SS and updates the RTC.
void RtcManager::serialSetupTime() {
    if (!Serial) Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println("📅 Checking RTC...");

    String nowTime = getFormattedTime();
    if (nowTime.startsWith("1970") || nowTime.startsWith("2000")) {
        Serial.println("❗ RTC time invalid or not set.");
        Serial.println("Please enter current date and time:");
        Serial.println("Format: YYYY-MM-DD HH:MM:SS");

        String input = "";
        while (input.length() < 19) {
            if (Serial.available()) {
                char c = Serial.read();
                if (c == '\n' || c == '\r') break;
                input += c;
                Serial.print(c);
            }
        }
        Serial.println();

        int y, M, d, h, m, s;
        if (sscanf(input.c_str(), "%d-%d-%d %d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
            setTime(y, M, d, h, m, s);
            Serial.println("✅ RTC updated successfully!");
        } else {
            Serial.println("❌ Invalid format. RTC not updated.");
        }
    } else {
        Serial.println("✅ RTC is already set: " + nowTime);
    }
}
