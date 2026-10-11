# ESP32 QR Payment Display 🚀

An IoT-based dynamic or static QR code payment display system powered by an **ESP32** microcontroller. This project is designed to fetch and display payment QR codes (compatible with local/global payment gateways like Fonepay, Khalti, Esewa, UPI, etc.) on an LCD/OLED display, making it an affordable, smart solution for merchants and small businesses.

---

## ✨ Features
- **ESP32 Powered:** Utilizes Wi-Fi capabilities for remote updates, configuration, or fetching dynamic amounts.
- **Clear Display:** Renders clear QR codes on compatible displays (e.g., TFT LCD / OLED / e-Paper).
- **Easy Configuration:** Simple Wi-Fi credentials setup (using Wi-Fi Manager or hardcoded variables).
- **Lightweight & Cost-Effective:** Built using readily available, budget-friendly microcontrollers and display modules.
- **Merchant Friendly:** Ideal for counter tops to receive digital wallet payments seamlessly.

---

## 🛠️ Hardware Requirements

| Component | Description | Quantity |
| :--- | :--- | :---: |
| **ESP32 Development Board** | NodeMCU-32S / ESP32-WROOM-32 | 1 |
| **Display Module** | e.g., 2.8" TFT LCD (ILI9341) / OLED / ST7789 | 1 |
| **Connecting Wires** | Male-to-Female / Breadboard jumper wires | As needed |
| **Power Source** | 5V Micro-USB / Type-C cable | 1 |

---

## 📐 Circuit / Pinout Connections



| ESP32 Pin | Display Module Pin | Description |
| :---: | :---: | :--- |
| **3V3** | VCC / LED | Power Supply (3.3V) |
| **GND** | GND | Ground |
| **GPIO 23** | MOSI (SDA) | SPI Data |
| **GPIO 18** | SCK (SCL) | SPI Clock |
| **GPIO 5** | CS | Chip Select |
| **GPIO 2** | DC / RS | Data/Command Control |
| **GPIO 4** | RST | Reset |

---

## 📦 Software & Libraries Required

Make sure you have the [Arduino IDE](https://www.arduino.cc/) installed with the **ESP32 board package** added.

### Required Arduino Libraries:
You can install these via the Arduino Library Manager (`Sketch` -> `Include Library` -> `Manage Libraries`):
1. **TFT_eSPI** (or the specific graphics library corresponding to your display model)
2. **QRCode** library by Richard Moore (for generating pixel matrices)
3. **ArduinoJson** (if parsing dynamic amounts via API/JSON)
