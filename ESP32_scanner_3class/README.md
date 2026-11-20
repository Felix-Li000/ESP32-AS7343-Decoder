# ESP32 Spectrally Encrypted QR Scanner (Three-Class)

This repository contains the complete embedded firmware for a **three-class spectral verification and QR scanning system** built on the ESP32. The system combines an AS7343 multi-channel spectral sensor, a 365 nm UV excitation source, an on-device Edge Impulse neural-network classifier, an SSD1306 OLED display, and a Bluetooth Low Energy (BLE) keyboard interface.

The classifier distinguishes three spectral states:

| Class | Meaning in the verification flow |
| --- | --- |
| `Background` | No valid sample is present. |
| `Encrypted` | Pristine (unstimulated) state of the spectral material. |
| `Decrypted` | State of the material after the external stimulus has been applied. |

Access is granted only when the spectral fingerprint changes from `Encrypted` to `Decrypted` across two consecutive, button-triggered acquisitions. After successful verification, QR codes may be scanned and their contents displayed or forwarded to a paired host as virtual keyboard input.

This codebase is intended to support reproducible research; all source comments and documentation are in English.

---

## Features

- On-device inference with an Edge Impulse Arduino library (no cloud connection required at run time).
- 208-dimensional input feature vector: 16 sequential acquisitions of 13 selected AS7343 channels each.
- Two-stage verification with a confidence threshold of 0.7 per stage.
- 0.96-inch SSD1306 OLED status display using the U8g2 graphics library.
- BLE HID keyboard emulation (`SpectraQR`) for driver-free output to a computer or smartphone.
- UART2-based QR scanner interface (MD500) and reserved UART1 forwarding for external logging.
- PlatformIO-based build with all dependencies declared in `platformio.ini`.

---

## Repository Structure

```
ESP32_scanner_3class/
├── src/
│   ├── main.cpp            # Application firmware: acquisition, inference, verification, display, BLE
│   └── BleKeyboard.cpp     # BLE HID keyboard implementation
├── include/
│   └── BleKeyboard.h       # BLE HID keyboard declarations
├── lib/
│   ├── ESP32-AS7343_Decoder_3class_inferencing/
│   │                       # Exported Edge Impulse Arduino library (3-class model)
│   └── ETHClass/           # Auxiliary local library placeholder
├── platformio.ini          # PlatformIO environment and dependency configuration
├── test/                   # PlatformIO test directory (reserved)
└── README.md               # This document
```

---

## Hardware Requirements

| Component | Notes |
| --- | --- |
| ESP32 development board | `esp32dev` form factor; tested with a 30-pin DevKit board |
| AMS AS7343 spectral sensor | 16-channel, I²C interface |
| 365 nm UV LED | Attached to the AS7343 breakout; used for excitation |
| QR code scanner module | MD500 or compatible UART scanner (57600 baud, 8N1) |
| OLED display | 0.96 inch, SSD1306 driver, 128×64 pixels |
| Four-button analog ladder | Connected to ADC input on GPIO4 |
| USB cable | Type-C, used for power, flashing, and UART0 debug output |
| Optional: USB-Bluetooth adapter | Required only for desktop hosts without built-in Bluetooth |

### Pin Mapping

| Signal | ESP32 GPIO | Direction / Notes |
| --- | --- | --- |
| I²C SCL | 22 | AS7343 and OLED clock (software I²C) |
| I²C SDA | 21 | AS7343 and OLED data (software I²C) |
| UV LED control | 32 | Output, active high |
| Scanner trigger | 33 | Output, active low |
| Scanner UART TX | 17 | ESP32 TX2 → scanner RXD |
| Scanner UART RX | 16 | ESP32 RX2 ← scanner TXD |
| Button ladder | 4 | Analog input (10-bit ADC) |
| Onboard LED | 2 | Status indicator |
| Debug UART (TX) | 1 | UART0, 115200 baud |
| Reserved UART1 | 5 (RX), 4 (TX) | 9600 baud, disabled by default |

> Note: GPIO4 is shared by the button ladder and the reserved UART1 TX line. UART1 forwarding is therefore commented out in the default firmware; enable it only if your wiring does not use GPIO4 as an analog input.

---

## Software Requirements

- [PlatformIO Core](https://platformio.org/) (CLI or IDE extension)
- Python 3 (required by PlatformIO)
- Git (for cloning this repository)

Dependencies are resolved automatically from `platformio.ini`:

- `olikraus/U8g2` (^2.35.4)
- `NimBLE-Arduino`
- `larryli/u8g2_wqy` (CJK font for U8g2)
- `adafruit/Adafruit BusIO` (^1.16.1)
- The exported Edge Impulse inference library, already included under `lib/`

---

## Build and Flash

```bash
git clone <repository-url>
cd ESP32_scanner_3class
pio run -e esp32dev -t upload
pio device monitor -e esp32dev
```

Alternatively, open the project folder in PlatformIO IDE, select the `esp32dev` environment, and click **Upload**.

The serial monitor runs at **115200 baud**. During inference, the firmware prints the confidence of each of the three classes to UART0.

---

## Operating Procedure

1. Power the board and wait for the welcome screen on the OLED display.
2. Pair your computer or smartphone with the BLE device named **`SpectraQR`**.
3. Press the verification button once to run **stage 1**. The UV LED turns on, 208 spectral features are acquired, and the classifier must return `Encrypted` with confidence greater than 0.7.
4. Apply the external stimulus to the material, then press the button again to run **stage 2**. This time the classifier must return `Decrypted` with confidence greater than 0.7.
5. If both stages pass, the OLED reports `Access granted!` and the system enters the unlocked state. Any failure resets the flow to stage 1.
6. In the unlocked state, press the scan button to read a QR code with the UART scanner. The decoded content appears on the OLED. When the scanned string matches the configured SHA-1 digest (`strSHA1` in `src/main.cpp`), the firmware displays the corresponding authorized URL.
7. Press the reset button to return to the locked state and the welcome screen.

Bluetooth keyboard output (`bleKeyboard.println(...)` / `bleKeyboard.print(...)`) is intentionally commented out in the released firmware. Uncomment the relevant lines in `GetSerialStuff()` to enable typing of scan results into the paired host.

---

## Model and Data Notes

- Input: 16 consecutive reads of the AS7343 sensor; for each read, 13 of the 16 channels are reordered into a fixed layout, producing a `float features[208]` vector.
- Model: dense neural network exported by Edge Impulse as an Arduino library (`ESP32-AS7343_Decoder_3class_inferencing`).
- Output: three softmax scores labelled `Background`, `Encrypted`, and `Decrypted`.
- The inference library is vendored in `lib/` so that the firmware can be rebuilt without access to the original Edge Impulse project.

---

## License

No license has been specified yet. Before making the repository public, add a `LICENSE` file that matches the requirements of your institution and funding agency.
