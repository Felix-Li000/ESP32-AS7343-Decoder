============================================================
Supporting Information
Embedded Spectral Fingerprint Acquisition and AI Classification System
============================================================
S1. System Overview

To implement the spectral-fingerprint-synergistic cryptographic strategy described in the main text, a custom embedded acquisition and classification platform was developed.

The system integrates:

A multi-channel spectral sensor (AS7343)

An ESP32 microcontroller

A QR data interface module

OLED display module

Bluetooth HID transmission

On-device neural network classifier

This hardware–software co-designed system enables direct acquisition of high-dimensional spectral fingerprints without RGB compression, followed by embedded AI-based classification for encrypted information processing.

S2. Hardware Platform
Microcontroller

MCU: ESP32-WROOM-32
Development board: DOIT ESP32 DEVKIT V1
Framework: Arduino
Build system: PlatformIO

The ESP32 was selected due to:

Integrated Bluetooth capability

Adequate SRAM for AI inference

Dual-core architecture

High-speed I2C and UART support

Spectral Sensor

Sensor model: AS7343
Type: 13-channel spectral sensor
Interface: I2C

The sensor provides discrete channel-resolved spectral intensity signals covering visible to near-infrared wavelengths.

I2C configuration:

SDA → GPIO6
SCL → GPIO7

Initialization:

Wire.begin(6, 7);
S3. Spectral Channel Mapping

The spectral channels used in this work are:

VIS
F1 (395–415 nm)
F2 (415–435 nm)
FZ (440–460 nm)
F3 (465–485 nm)
F4 (505–525 nm)
F5 (540–566 nm)
FY (545–565 nm)
FXL (590–610 nm)
F6 (630–650 nm)
F7 (680–700 nm)
F8 (735–755 nm)
NIR (845–865 nm)

These multi-channel intensities form the high-dimensional spectral fingerprint input vector for classification.

S4. Embedded AI Classification

The acquired spectral fingerprint (multi-channel intensity vector) is processed by an embedded neural network model deployed on the ESP32.

Model characteristics:

Input: multi-channel spectral vector

Output: classification label

Inference mode: on-device

No external computation required

The AI classifier enables discrimination between encryption and decryption spectral states despite non-uniform spectral channel distribution and correlated spectral variations.

S5. QR Interaction and Output

A UART-based QR module is integrated to link spectral fingerprint classification with encrypted digital content.

UART configuration:

RX → GPIO16
TX → GPIO17
Baudrate: 115200

The final decoded result can be:

Displayed on OLED

Printed via serial port

Transmitted wirelessly using Bluetooth HID (keyboard emulation mode)

S6. System Operation Workflow

Spectral fingerprint acquisition

Multi-channel intensity normalization

AI classification

Cryptographic state determination

Output via display or Bluetooth

This design converts the decryption process into a spatiotemporally gated spectral processing operation, as described in the main manuscript.

S7. Reproducibility Notes

Integration time and gain must be optimized to avoid channel saturation

Ambient light conditions should be controlled

Bluetooth pairing required before use

PlatformIO configuration used in this work:

platform = espressif32
board = DOIT ESP32 DEVKIT V1
framework = arduino

S8. Research Context

This embedded system serves as the hardware realization of the molecule–electronics–AI interactive cryptographic strategy.

By preserving channel-resolved spectral fingerprints rather than compressing into RGB space, the system enables expansion of theoretical encryption dimensionality and robust AI-assisted classification.

END OF SUPPORTING DESCRIPTION