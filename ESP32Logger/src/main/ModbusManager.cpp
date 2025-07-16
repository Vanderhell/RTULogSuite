#include "ModbusManager.h"
#include <ModbusMaster.h>
#include <stack>
#include <vector>

// Static RX/TX pin assignments for hardware serial (UART1)
static const int MODBUS_RX_PIN = 17;
static const int MODBUS_TX_PIN = 16;
static int deRePin = 4;  // DE/RE pin for RS485 direction control
static ConfigManager* config = nullptr;
static ModbusMaster node; // Global ModbusMaster instance

/// <summary>
/// Converts a Modbus error code into a human-readable string.
/// </summary>
const char* ModbusManager::modbusErrorToStr(uint8_t code) {
    switch (code) {
        case 0x00: return "Success";
        case 0x01: return "Illegal Function";
        case 0x02: return "Illegal Data Address";
        case 0x03: return "Illegal Data Value";
        case 0x04: return "Slave Device Failure";
        case 0x05: return "Acknowledge";
        case 0x06: return "Slave Device Busy";
        case 0x08: return "Memory Parity Error";
        case 0x0A: return "Gateway Path Unavailable";
        case 0x0B: return "Gateway Target Device Failed to Respond";
        case 0xE0: return "Invalid Slave ID";
        case 0xE1: return "Invalid Function";
        case 0xE2: return "Response Timeout";
        case 0xE3: return "Invalid CRC";
        case 0xE4: return "Bad response length";
        default:   return "Unknown error";
    }
}

/// <summary>
/// Injects a pointer to the current configuration (needed for offset/scaling logic).
/// </summary>
void ModbusManager::setConfig(ConfigManager* cfg) {
    config = cfg;
}

/// <summary>
/// RS485 pre-transmission callback: sets DE/RE pin HIGH to enable transmit.
/// </summary>
void preTransmission() {
    if (deRePin >= 0)
        digitalWrite(deRePin, HIGH);
}

/// <summary>
/// RS485 post-transmission callback: sets DE/RE pin LOW to enable receive.
/// </summary>
void postTransmission() {
    if (deRePin >= 0)
        digitalWrite(deRePin, LOW);
}

/// <summary>
/// Initializes UART1 and ModbusMaster for Modbus RTU communication over RS485.
/// </summary>
void ModbusManager::begin(const ModbusSettings& settings) {
    Serial.println("[ModbusManager] Initializing UART1 for RS485...");
    Serial.printf("  - RX pin: %d, TX pin: %d, DE/RE pin: %d\n", MODBUS_RX_PIN, MODBUS_TX_PIN, deRePin);
    Serial.printf("  - Baudrate: %ld, Parity: %c, Stop Bits: %d, Data Bits: %d\n",
                  settings.baudrate, settings.parity, settings.stop_bits, settings.data_bits);

    Serial1.begin(settings.baudrate, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
    pinMode(deRePin, OUTPUT);
    digitalWrite(deRePin, LOW);

    node.begin(settings.slave_id, Serial1);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);

    Serial.printf("[ModbusManager] Modbus slave ID set to %d\n", settings.slave_id);
    Serial.println("[ModbusManager] Modbus interface initialized successfully.");
}

/// <summary>
/// Sets voltage and current transformer ratios used in scaling expressions.
/// </summary>
void ModbusManager::setTransformers(float vtr, float ctr) {
    currentVTR = vtr;
    currentCTR = ctr;
    Serial.printf("[ModbusManager] Set VTR = %.3f, CTR = %.3f\n", currentVTR, currentCTR);
}

// Operator precedence (for + - * /)
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Apply an operation between two float values
float applyOp(float a, float b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0.0f) ? a / b : NAN;
        default: return NAN;
    }
}

/// <summary>
/// Evaluates a simple math expression with variables: val, VTR, CTR.
/// Example: "val * 0.1 * CTR / VTR"
/// </summary>
float evaluateExpression(String expr, float val, float VTR, float CTR) {
    expr.replace("val", String(val, 6));
    expr.replace("VTR", String(VTR, 6));
    expr.replace("CTR", String(CTR, 6));
    expr.replace(" ", "");

    std::stack<float> values;
    std::stack<char> ops;
    int i = 0;

    while (i < expr.length()) {
        char c = expr[i];

        if (isdigit(c) || c == '.') {
            String numStr;
            while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) {
                numStr += expr[i++];
            }
            values.push(numStr.toFloat());
            continue;
        }

        if (c == '(') {
            ops.push(c);
        } else if (c == ')') {
            while (!ops.empty() && ops.top() != '(') {
                if (values.size() < 2) return NAN;
                float b = values.top(); values.pop();
                float a = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(a, b, op));
            }
            if (!ops.empty()) ops.pop();
        } else if (c == '+' || c == '-' || c == '*' || c == '/') {
            while (!ops.empty() && precedence(ops.top()) >= precedence(c)) {
                if (values.size() < 2) return NAN;
                float b = values.top(); values.pop();
                float a = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(a, b, op));
            }
            ops.push(c);
        }
        i++;
    }

    while (!ops.empty()) {
        if (values.size() < 2) return NAN;
        float b = values.top(); values.pop();
        float a = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        values.push(applyOp(a, b, op));
    }

    return values.empty() ? NAN : values.top();
}

/// <summary>
/// Reads and scales all configured registers.
/// If a read fails, NAN is inserted in place of the value.
/// </summary>
std::vector<float> ModbusManager::readAll(const std::vector<RegisterConfig>& regs) {
    std::vector<float> results;
    Serial.println("[ModbusManager] Starting Modbus read of all configured registers...");

    for (const auto& reg : regs) {
        uint16_t modbusAddress = reg.register_address;
        if (config && config->isAddressOffsetEnabled()) {
            modbusAddress -= 1;
        }

        Serial.printf("  > Reading [%s] @ %u (%s)... ", reg.key.c_str(), modbusAddress, reg.type.c_str());

        uint8_t result = node.readHoldingRegisters(modbusAddress, reg.length);

        if (result == node.ku8MBSuccess) {
            uint16_t raw = node.getResponseBuffer(0);
            Serial.printf("    ↪ Scaling expr: %s | val = %u | VTR = %.2f | CTR = %.2f\n",
                reg.scaling.c_str(), raw, currentVTR, currentCTR);
            float scaledValue = evaluateExpression(reg.scaling, raw, currentVTR, currentCTR);
            results.push_back(scaledValue);
            Serial.printf("OK (raw = %d → scaled = %.3f)\n", raw, scaledValue);
        } else {
            results.push_back(NAN);
            Serial.printf("FAIL (code 0x%02X = %s)\n", result, modbusErrorToStr(result));
        }
    }

    Serial.printf("[ModbusManager] Finished Modbus read. %d value(s) retrieved.\n", results.size());
    return results;
}

/// <summary>
/// Reads a single Modbus register and stores the raw 16-bit value.
/// Applies offset if enabled.
/// </summary>
bool ModbusManager::readRegister(uint16_t reg, uint16_t* outValue) {
    uint16_t address = reg;
    if (config && config->isAddressOffsetEnabled()) {
        address -= 1;
    }

    uint8_t result = node.readHoldingRegisters(address, 1);
    if (result == node.ku8MBSuccess) {
        *outValue = node.getResponseBuffer(0);
        return true;
    } else {
        Serial.printf("[ModbusManager][ERROR] ❌ Reading register %u failed: code 0x%02X (%s)\n",
                      reg, result, modbusErrorToStr(result));
        return false;
    }
}

/// <summary>
/// Scans a range of Modbus registers and prints values for debugging.
/// Optionally applies offset correction.
/// </summary>
void ModbusManager::scanRange(uint16_t start, uint16_t end, bool applyOffset) {
    bool useOffset = applyOffset && config && config->isAddressOffsetEnabled();

    Serial.printf("\n🔍 [ModbusManager] Scanning Modbus registers from %u to %u (offset: %s)...\n",
                  start, end, useOffset ? "enabled" : "disabled");

    for (uint16_t addr = start; addr <= end; ++addr) {
        uint16_t realAddr = useOffset ? addr - 1 : addr;

        uint8_t result = node.readHoldingRegisters(realAddr, 1);
        if (result == node.ku8MBSuccess) {
            uint16_t value = node.getResponseBuffer(0);
            Serial.printf("  ✅ Reg %u [%u] → %u (0x%04X)\n", addr, realAddr, value, value);
        } else {
            Serial.printf("  ❌ Reg %u [%u] → Read failed (0x%02X = %s)\n",
                          addr, realAddr, result, modbusErrorToStr(result));
        }
        delay(10);
    }

    Serial.println("🔍 [ModbusManager] Scan completed.\n");
}
