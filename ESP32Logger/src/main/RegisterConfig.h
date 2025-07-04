#ifndef REGISTER_CONFIG_H
#define REGISTER_CONFIG_H

#include <Arduino.h>

// Describes a single Modbus register or measurement parameter.
// Used to configure registers dynamically from a JSON file.
// Includes metadata, access control, and value scaling.
struct RegisterConfig {
    String key;                // Unique key identifier (used in code and logs)
    String name;               // Human-readable name (used in UI or CSV headers)
    String description;        // Full description of the parameter for users or diagnostics
    uint16_t register_address; // Modbus address (starting from 0 or 1 depending on device)
    String type;               // Data type (e.g., "UINT16", "FLOAT", "UINT32")
    String unit;               // Measurement unit (e.g., "V", "A", "Hz", "-")
    String scaling;            // Scaling expression (e.g., "val * 0.1", "CTR * val / VTR")
    String access;             // Access mode ("R-only", "R/W")
    uint8_t length;            // Number of 16-bit Modbus registers (1 for 16-bit, 2 for 32-bit values)
};

#endif // REGISTER_CONFIG_H
