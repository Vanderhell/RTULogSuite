#include "SystemManager.h"

// Global instance of the system manager
SystemManager systemManager;

// Arduino setup function
// Called once after power-up or reset
void setup() {
    // Initialize all system components (RTC, SD card, config, Modbus, etc.)
    systemManager.setupAll();
}

// Arduino main loop function
// Called repeatedly after setup()
void loop() {
    // Perform one data acquisition and logging cycle
    systemManager.runCycle();

    // Delay to avoid overloading Modbus or SD card
    delay(100);
}
