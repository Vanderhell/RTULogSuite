#include "SystemManager.h"
#include "RtcManager.h"
#include "ConfigManager.h"
#include "StorageManager.h"
#include "ModbusManager.h"
#include "DataLogger.h"

/// <summary>
/// Returns the configured polling interval from the loaded configuration.
/// </summary>
unsigned long SystemManager::getPollingInterval() const {
    return config.getPollingInterval();
}

/// <summary>
/// Triggers RTC time setup via serial input.
/// Useful when RTC has invalid or unset time.
/// </summary>
void SystemManager::setupRTCFromSerial() {
    rtc.serialSetupTime();
}

/// <summary>
/// Performs complete system initialization.
/// Called once during startup to initialize:
/// - RTC (including manual time entry if invalid)
/// - SD card for storage
/// - Configuration loading
/// - Modbus communication
/// - Transformer register readout (VTR/CTR)
/// - DataLogger instance
/// </summary>
void SystemManager::setupAll() {
    Serial.println("🔧 [SystemManager] Starting system setup...");

    // 1. RTC Initialization
    Serial.println("⏱️  [SystemManager] Initializing RTC...");
    rtc.begin();
    if (rtc.getFormattedTime().startsWith("1970") || rtc.getFormattedTime().startsWith("2000")) {
        Serial.println("⚠️  [SystemManager] RTC time invalid → prompting manual setup.");
        rtc.serialSetupTime();
    } else {
        Serial.println("✅ [SystemManager] RTC time is valid: " + rtc.getFormattedTime());
    }

    // 2. SD card setup
    Serial.println("💾 [SystemManager] Initializing storage...");
    storage.begin();

    // 3. Configuration load
    Serial.println("🗂️  [SystemManager] Loading configuration...");
    config.setStorage(&storage);
    config.load();

    // 4. Modbus setup
    ModbusSettings mb = config.getModbusSettings();
    modbus.begin(mb);
    modbus.setConfig(&config);
    modbus.setAddressOffset(config.isAddressOffsetEnabled());

    // 5. Read VTR and CTR registers
    uint16_t vtrAddr = config.getVTRRegister();
    uint16_t ctrAddr = config.getCTRRegister();
    uint16_t vtrRaw = 0, ctrRaw = 0;
    float vtr = config.getVTR();  // fallback from config
    float ctr = config.getCTR();

    // Optional debug scan of registers
    if (config.isDebugEnabled()) {
        bool offset = config.isAddressOffsetEnabled();
        modbus.scanRange(4000, 4100, offset);
    }

    // Read raw transformer values if addresses are defined
    if (vtrAddr > 0 && ctrAddr > 0) {
        Serial.printf("[SystemManager] Attempting to read VTR @ %u and CTR @ %u...\n", vtrAddr, ctrAddr);

        bool vtrOk = modbus.readRegister(vtrAddr, &vtrRaw);
        bool ctrOk = modbus.readRegister(ctrAddr, &ctrRaw);

        if (vtrOk && ctrOk) {
            vtr = static_cast<float>(vtrRaw);
            ctr = static_cast<float>(ctrRaw);

            Serial.printf("[SystemManager] ✅ VTR register raw = %u → interpreted = %.2f\n", vtrRaw, vtr);
            Serial.printf("[SystemManager] ✅ CTR register raw = %u → interpreted = %.2f\n", ctrRaw, ctr);
        } else {
            Serial.printf("[SystemManager][WARN] ❌ Failed to read:\n");
            if (!vtrOk)
                Serial.printf("  - VTR @ %u failed.\n", vtrAddr);
            if (!ctrOk)
                Serial.printf("  - CTR @ %u failed.\n", ctrAddr);

            Serial.printf("[SystemManager] ⚠️ Using fallback values from config.json → VTR = %.2f, CTR = %.2f\n", vtr, ctr);
        }
    } else {
        Serial.println("[SystemManager] Transformer register addresses not defined. Using config.json values.");
    }

    // Apply transformer ratios to Modbus manager
    Serial.printf("[DEBUG] Final transformer ratios → VTR = %.2f, CTR = %.2f\n", vtr, ctr);
    modbus.setTransformers(vtr, ctr);

    // 6. Create data logger
    Serial.println("📝 [SystemManager] Creating DataLogger instance...");
    logger = DataLogger(&rtc, &storage, &modbus, &config);

    Serial.println("✅ [SystemManager] System setup complete.");
}

/// <summary>
/// Executes a single measurement and logging cycle.
/// Checks for SD card availability, performs Modbus readout, and logs values.
/// Should be called periodically in the main loop.
/// </summary>
void SystemManager::runCycle() {
    Serial.println("🔁 [SystemManager] Starting run cycle...");

    if (storage.isCardPresent()) {
        logger.logAll();
    } else {
        Serial.println("❌ [SystemManager] SD card not detected!");
        storage.logError("SD card not present.");
    }

    Serial.println("✅ [SystemManager] Run cycle complete.\n");
}
