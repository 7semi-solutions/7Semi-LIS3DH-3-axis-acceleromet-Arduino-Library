# 7Semi LIS3DH Library

Arduino library for the **LIS3DH 3-axis accelerometer** (STMicroelectronics).

This library supports both **I2C and SPI** communication and makes it easy to read:

- Acceleration values (raw and g-units)
- Threshold interrupts (INT1 and INT2)
- ADC channel readings (ADC1 / ADC2 / ADC3)
- Temperature output from internal sensor

The goal of this library is to keep **setup simple**, while still giving access to advanced LIS3DH features.

---

## Why use LIS3DH?

LIS3DH is a popular accelerometer used in many applications:

- Motion detection
- Step counting
- Tilt / orientation sensing
- Fall detection
- Vibration monitoring
- Wearables and fitness devices
- Industrial motion monitoring

It supports:
- Very low power mode
- High resolution mode
- Interrupt based detection (no polling needed)

---

## Features

- **I2C + SPI communication**  
- **Acceleration output**
- Raw signed 16-bit values
- Converted into g-units (float)

- **Sensor configuration**
- Scale selection (±2g / ±4g / ±8g / ±16g)
- Output Data Rate selection (1 Hz to 1.6 kHz)

- **Interrupt support**
- INT1 threshold interrupts
- INT2 threshold interrupts
- Polarity control (active high / low)

- **ADC + Temperature**
- 3 ADC channels available
- Temperature output via ADC3

---

## Supported Boards

- Arduino UNO / Mega / Nano
- ESP32 / ESP8266
- Any Arduino-compatible board with I2C or SPI

---

## Communication Modes

The sensor supports both I2C and SPI.

### I2C Mode
- Uses only **2 wires** (SDA + SCL)
- Slower than SPI, but simple
- Default address: `0x18`
- Alternate address: `0x19` (depends on SA0 pin)

### SPI Mode
- Uses **4 wires + CS**
- Faster reads, best for higher performance applications
- Requires manual CS pin control
- SPI mode is fixed to `SPI_MODE3` for LIS3DH

---

## Wiring

### I2C Wiring
| LIS3DH Pin | Arduino Pin |
|----------:|------------:|
| VCC       | 3.3V / 5V   |
| GND       | GND         |
| SDA       | SDA         |
| SCL       | SCL         |

> If using a 5V board (UNO), confirm that your module has level shifting or supports 5V logic.

---

### SPI Wiring
| LIS3DH Pin | Arduino Pin |
|----------:|------------:|
| VCC       | 3.3V / 5V   |
| GND       | GND         |
| SCL       | SCK         |
| SDO      | MOSI        |
| SDI      | MISO        |
| CS        | Any GPIO    |

---

### Interrupt Wiring (Optional)
| LIS3DH Pin | Arduino Pin |
|----------:|------------:|
| INT1      | D2 (example)|
| INT2      | D3 (example)|

INT pins are useful for:
- Motion detection
- Wake-up events
- Threshold triggers
- Free-fall events (can be implemented using interrupts)

---

## Installation

### Arduino Library Manager
1. Arduino IDE → **Tools → Manage Libraries**
2. Search: **7Semi LIS3DH**
3. Click install

### Manual Install
1. Download repository ZIP
2. Arduino IDE → **Sketch → Include Library → Add .ZIP Library**
3. Restart IDE

---

## Basic Usage Flow

Most users follow this pattern:

1. **Initialize** (I2C or SPI)
2. **Configure scale and data rate**
3. **Read acceleration**
4. Optional: use interrupts for event detection
5. Optional: read ADC and temperature

---
