# EM-07 Modbus Data Logger

A modular ESP32-based Modbus RTU data logger with support for SD card logging, JSON configuration, real-time clock (RTC), and dynamic scaling of transformer-based values.

---

## 🔧 Features
- Reads data from Modbus RTU devices (e.g., EM-07 meters)
- Dynamically configurable via `config.json`
- Logs data to SD card in JSON or CSV format
- Timestamped data using DS3231 RTC
- Interactive RTC setup via Serial
- Easy to extend with additional registers or logic

---

## 📂 File Structure

| File                  | Purpose                                |
|-----------------------|----------------------------------------|
| `config.json`         | Main configuration file for registers  |
| `main.ino`            | Arduino entry point                    |
| `SystemManager.*`     | Startup and runtime orchestration      |
| `RtcManager.*`        | RTC time handling                      |
| `ConfigManager.*`     | Loads `config.json`                    |
| `ModbusManager.*`     | Handles Modbus requests & scaling      |
| `StorageManager.*`    | Logging to SD card                     |
| `DataLogger.*`        | Ties together config, Modbus, and logs |

---

## 🗂️ config.json
A human-readable file that defines Modbus registers to read, how to scale them, and logging settings.

See [`config_json_documentation`](./config_json_documentation.md) for full format description.

### Example:
```json
{
  "device": "EM-07",
  "transformers": {
    "VTR": 100,
    "CTR": 200
  },
  "logging": {
    "interval_ms": 1000
  },
  "registers": [ ... ]
}
```

---

## 📌 Hardware Requirements
- **ESP32 board** (tested on DOIT ESP32 DEVKIT V1)
- **DS3231 RTC module** (I2C)
- **RS485 module** for Modbus communication
- **SD card module** (SPI)

> ⚠️ **Pinout must be set in code. Not all hardware pins are defined in `config.json`.**
>
> Check these inside `ModbusManager.cpp`, `StorageManager.cpp`, or `main.ino`
>
> Example for RS485:
> ```cpp
> static const int MODBUS_RX_PIN = 17;
> static const int MODBUS_TX_PIN = 16;
> static int deRePin = 4; // <- manually set here
> ```

---

## 🔌 Setting the RTC Time

### Option A: Automatically Prompted on First Boot
If the RTC returns an invalid time (e.g., 1970 or 2000), the system prompts you via Serial to enter the correct datetime.

#### Format required:
```
YYYY-MM-DD HH:MM:SS
```

#### Example:
```
2025-07-03 14:30:00
```

### Option B: Force via setup pin (optional)
You can connect a button to GPIO (e.g. GPIO0) to trigger setup mode manually.

---

## 🖥️ serialSetupMenu() [optional]

An interactive serial console with basic setup/testing features:
- [1] Set RTC Time
- [2] Show current config
- [3] Print register test read
- [0] Exit

> Add this menu to `loop()` if needed:
```cpp
if (Serial.available()) {
    serialSetupMenu();
}
```

---

## 🚀 Getting Started (Arduino IDE)

### 1. Install dependencies:
- ESP32 board support via Board Manager
- Libraries:
  - `RTClib`
  - `ArduinoJson`
  - `ModbusMaster`
  - `SD`

### 2. Flashing
- Open `main.ino` in Arduino IDE
- Select correct board + COM port
- Flash the project

### 3. Initial Setup
- Insert SD card with `config.json`
- Connect device via Serial Monitor (baud 115200)
- If prompted, enter current date/time

---

## 📎 Notes
- Ensure your Modbus slave responds correctly to the register map.
- Double-check `length` field in registers (1 = 16-bit, 2 = 32-bit)
- Use `scaling` expressions like `val * 0.1 * VTR`

---

## 📃 License
MIT License


