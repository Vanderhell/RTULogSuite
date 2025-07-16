#include "SystemManager.h"
#include "ConfigManager.h"
#include "DataLogger.h"

SystemManager systemManager;

unsigned long lastPollTime = 0;
unsigned long pollInterval = 1000;

/// <summary>
/// Arduino setup() function.
/// Initializes system components, loads configuration,
/// and optionally prints debug information to Serial.
/// </summary>
void setup() {
    Serial.begin(115200);
    delay(1000); // Short delay to allow Serial to initialize

    Serial.println("=== ESP32 Modbus Logger Start ===");

    // Initialize all subsystems (RTC, SD, config, Modbus, logger)
    systemManager.setupAll();
    Serial.println("✅ System setup done.");

    // Optional debug output
    if (systemManager.getConfig()->isDebugEnabled()) {
        Serial.println("[Debug] Enabled.");

        // Show Modbus settings
        ModbusSettings s = systemManager.getConfig()->getModbusSettings();
        Serial.println("[Debug] Modbus Communication Settings:");
        Serial.printf("  - Modbus ID: %d\n", s.slave_id);
        Serial.printf("  - Baudrate: %ld\n", s.baudrate);
        Serial.printf("  - Parity: %c\n", s.parity);
        Serial.printf("  - Stop Bits: %d\n", s.stop_bits);
        Serial.printf("  - Data Bits: %d\n", s.data_bits);

        // Show register list
        auto regs = systemManager.getConfig()->getRegisters();
        Serial.printf("[Debug] Loaded %d register(s):\n", regs.size());
        for (const auto& r : regs) {
            Serial.printf("  - %s [%s] @%d (%s), scaling: %s\n",
                          r.key.c_str(), r.name.c_str(),
                          r.register_address, r.type.c_str(), r.scaling.c_str());
        }

        // Show RTC time
        String now = systemManager.getRtc()->getFormattedTime();
        Serial.printf("[Debug] RTC Time: %s\n", now.c_str());
    }

    // Set polling interval from config
    pollInterval = systemManager.getPollingInterval();

    Serial.println("=== Setup Complete ===\n");
}

/// <summary>
/// Arduino loop() function.
/// Periodically triggers a data logging cycle and handles serial commands.
/// </summary>
void loop() {
    unsigned long now = millis();

    checkSerialCommands();

    if (now - lastPollTime >= pollInterval) {
        systemManager.runCycle();
        lastPollTime = now;
    }

    delay(10); // Allow CPU a short rest
}

/// <summary>
/// Handles user input via Serial.
/// Currently supports:
/// - setrtc → manually updates RTC via Serial terminal
/// </summary>
void checkSerialCommands() {
    static String input = "";

    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            input.trim();
            if (input.equalsIgnoreCase("setrtc")) {
                systemManager.setupRTCFromSerial();
            }
            input = "";
        } else {
            input += c;
        }
    }
}
