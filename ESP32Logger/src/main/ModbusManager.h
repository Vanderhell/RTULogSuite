#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include "ConfigManager.h"

/// <summary>
/// Handles Modbus RTU communication and scaling of raw register values.
/// Responsible for:
/// - UART and ModbusMaster initialization
/// - Reading and decoding registers
/// - Applying custom scaling formulas (e.g., "val * 0.1 * VTR")
/// </summary>
class ModbusManager {
public:
    /// <summary>
    /// Scans a range of Modbus registers and prints values via Serial for debugging.
    /// Useful for reverse-engineering unknown devices or diagnostics.
    /// </summary>
    /// <param name="start">Start address</param>
    /// <param name="end">End address</param>
    /// <param name="applyOffset">Whether to apply address offset (0-based vs 1-based)</param>
    void scanRange(uint16_t start, uint16_t end, bool applyOffset = false);

    /// <summary>
    /// Converts Modbus error code to human-readable description string.
    /// </summary>
    /// <param name="code">Modbus error code</param>
    /// <returns>C-string description of the error</returns>
    static const char* modbusErrorToStr(uint8_t code);

    /// <summary>
    /// Initializes the Modbus communication interface.
    /// Sets up UART and ModbusMaster with parameters from ModbusSettings.
    /// </summary>
    /// <param name="settings">Modbus configuration parameters</param>
    void begin(const ModbusSettings& settings);

    /// <summary>
    /// Sets transformer scaling ratios.
    /// These are injected into formulas for voltage/current scaling (e.g., VTR/CTR).
    /// </summary>
    /// <param name="vtr">Voltage transformer ratio</param>
    /// <param name="ctr">Current transformer ratio</param>
    void setTransformers(float vtr, float ctr);

    /// <summary>
    /// Reads all Modbus registers defined in the given configuration list.
    /// Returns scaled float values using expressions from RegisterConfig.
    /// </summary>
    /// <param name="regs">List of register configurations</param>
    /// <returns>Vector of float results, NAN if failed</returns>
    std::vector<float> readAll(const std::vector<RegisterConfig>& regs);

    /// <summary>
    /// Reads a single Modbus register and stores its raw 16-bit value.
    /// </summary>
    /// <param name="reg">Register address</param>
    /// <param name="outValue">Pointer to store the result</param>
    /// <returns>True if successful, false if failed</returns>
    bool readRegister(uint16_t reg, uint16_t* outValue);

    /// <summary>
    /// Enables or disables address offset correction (for 1-based or 0-based addressing).
    /// </summary>
    void setAddressOffset(bool enabled) { addressOffsetEnabled = enabled; }

    /// <summary>
    /// Injects reference to the global configuration object.
    /// Used for evaluating scaling expressions that reference config values.
    /// </summary>
    void setConfig(ConfigManager* cfg);

private:
    float currentVTR = 1.0f;            ///< Voltage transformer ratio
    float currentCTR = 1.0f;            ///< Current transformer ratio
    bool addressOffsetEnabled = false; ///< Whether to apply address offset (+1)
};

#endif // MODBUS_MANAGER_H
