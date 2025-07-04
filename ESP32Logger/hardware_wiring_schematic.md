# 🧰 EM-07 Logger – Hardware Wiring Schematic

This document describes the exact wiring for your hardware setup, based on the following components:

- SD Card module (3.3V logic)
- DS3231 RTC module (I2C)
- RS485 TTL module (Modbus RTU)
- ESP32 DevKit board

---

## 📦 Wiring Table

### ESP32 → SD Card Module

| SD Module Pin | Connect to ESP32 | Function               |
|---------------|------------------|------------------------|
| `3.3V`        | `3V3`            | Power supply           |
| `GND`         | `GND`            | Ground                 |
| `MISO`        | `GPIO19`         | SPI MISO               |
| `MOSI`        | `GPIO23`         | SPI MOSI               |
| `CLK`         | `GPIO18`         | SPI Clock              |
| `CS`          | `GPIO5`          | SPI Chip Select        |

### ESP32 → DS3231 RTC Module

| RTC Pin  | Connect to ESP32 | Function               |
|----------|------------------|------------------------|
| `VCC`    | `3V3`            | Power supply           |
| `GND`    | `GND`            | Ground                 |
| `SDA`    | `GPIO21`         | I2C data               |
| `SCL`    | `GPIO22`         | I2C clock              |

### ESP32 → RS485 TTL Module

| RS485 Pin | Connect to ESP32 | Function                                  |
|-----------|------------------|-------------------------------------------|
| `VCC`     | `3V3` or `5V`     | Power (based on module compatibility)     |
| `GND`     | `GND`            | Ground                                    |
| `RXD`     | `GPIO17`         | ESP32 TX → RS485 RX (Modbus TX)          |
| `TXD`     | `GPIO16`         | ESP32 RX ← RS485 TX (Modbus RX)          |

> 💡 If your RS485 module has a `DE` or `RE` pin, connect it to a free GPIO (e.g. `GPIO4`) and set it HIGH before transmission and LOW after.

---

## 🔧 Pin Definitions in Code

### ModbusManager.cpp
```cpp
static const int MODBUS_RX_PIN = 16;
static const int MODBUS_TX_PIN = 17;
static int deRePin = 4; // RS485 direction control pin (optional)
```

### SD Card Initialization
```cpp
SD.begin(5); // GPIO5 as CS pin
```

---

## 📝 Notes
- Ensure all grounds (`GND`) are connected between modules and ESP32.
- Use level shifters if your RS485 module is not 3.3V compatible.
- SD card must be formatted as FAT32.
- DS3231 requires pull-up resistors on SDA/SCL (typically already on module).

---

