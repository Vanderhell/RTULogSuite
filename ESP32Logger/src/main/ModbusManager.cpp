#include "ModbusManager.h"
#include <ModbusMaster.h>
#include <stack>
#include <vector>

// Static RX/TX pin assignments for hardware serial
static const int MODBUS_RX_PIN = 17;
static const int MODBUS_TX_PIN = 16;
static int deRePin = -1;  // Pin for DE/RE (RS485 direction control), disabled if < 0

static ModbusMaster node; // Global ModbusMaster instance

// Called before each Modbus transmission to enable RS485 driver
void preTransmission() {
    if (deRePin >= 0)
        digitalWrite(deRePin, HIGH);
}

// Called after each Modbus transmission to disable RS485 driver
void postTransmission() {
    if (deRePin >= 0)
        digitalWrite(deRePin, LOW);
}

// Initializes the Modbus communication interface
// Sets up UART1 for RS485 and configures ModbusMaster instance
void ModbusManager::begin(const ModbusSettings& settings) {
    Serial1.begin(settings.baudrate, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
    pinMode(deRePin, OUTPUT);
    digitalWrite(deRePin, LOW);

    node.begin(settings.slave_id, Serial1);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
}

// Set the transformer scaling ratios used in value calculations
void ModbusManager::setTransformers(float vtr, float ctr) {
    currentVTR = vtr;
    currentCTR = ctr;
}

// Returns precedence level of operators for expression evaluation
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Applies a binary arithmetic operation to two operands
float applyOp(float a, float b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0.0f ? a / b : NAN;
        default: return NAN;
    }
}

// Evaluate a scaling expression like "val * 0.1" or "CTR * val / VTR"
// Variables supported: val, VTR, CTR
// Uses simple infix-to-postfix parsing with operator precedence
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
                float b = values.top(); values.pop();
                float a = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(a, b, op));
            }
            if (!ops.empty()) ops.pop(); // pop '('
        } else if (c == '+' || c == '-' || c == '*' || c == '/') {
            while (!ops.empty() && precedence(ops.top()) >= precedence(c)) {
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
        float b = values.top(); values.pop();
        float a = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        values.push(applyOp(a, b, op));
    }

    return values.empty() ? NAN : values.top();
}

// Reads all registers defined in the configuration and returns their scaled values
// If reading fails, NAN is inserted for that value
std::vector<float> ModbusManager::readAll(const std::vector<RegisterConfig>& regs) {
    std::vector<float> results;

    for (const auto& reg : regs) {
        uint8_t result = node.readHoldingRegisters(reg.register_address, reg.length);

        if (result == node.ku8MBSuccess) {
            uint16_t raw = node.getResponseBuffer(0);
            float scaledValue = evaluateExpression(reg.scaling, raw, currentVTR, currentCTR);
            results.push_back(scaledValue);
        } else {
            results.push_back(NAN); // Failed read
        }
    }

    return results;
}
