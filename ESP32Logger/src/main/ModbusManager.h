#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include "ConfigManager.h"

// Handles Modbus RTU communication and scaling of raw register values.
// Responsible for initializing serial interface, reading values,
// and applying scaling expressions like "val * 0.1 * VTR".
class ModbusManager {
public:
    // Initialize Modbus communication.
    // Sets up UART1 and ModbusMaster with parameters like baudrate and parity.
    void begin(const ModbusSettings& settings);

    // Set transformer scaling ratios for voltage and current.
    // Used in scaling expressions such as "val * 0.1 * VTR".
    void setTransformers(float vtr, float ctr);

    // Read all given Modbus registers and return scaled float values.
    // Applies scaling formulas from config. Returns NAN if reading fails.
    std::vector<float> readAll(const std::vector<RegisterConfig>& regs);

private:
    float currentVTR = 1.0f; // Voltage transformer ratio
    float currentCTR = 1.0f; // Current transformer ratio
};

#endif // MODBUS_MANAGER_H
