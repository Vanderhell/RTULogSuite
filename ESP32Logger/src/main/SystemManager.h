#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

// Central manager that initializes and runs the embedded system.
// Coordinates initialization of RTC, SD card, configuration, and Modbus,
// and handles periodic data acquisition and logging.
class SystemManager {
public:
    // Perform system-wide initialization.
    // Should be called once during setup().
    void setupAll();

    // Run one full logging cycle.
    // Checks SD card presence and logs Modbus data.
    // Intended to be called periodically (e.g. in loop()).
    void runCycle();
};

#endif // SYSTEM_MANAGER_H
