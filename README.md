ESP32_AS7343_Decoder_Scanner

This repository contains the embedded implementation of a spectral-fingerprint–based cryptographic decoding platform integrating a 13-channel spectral sensor (AS7343), an ESP32 microcontroller, and an embedded neural network classifier trained via Edge Impulse.

The system enables real-time classification of stimulus-dependent spectral fingerprints and conditional QR-code decoding. It forms the hardware realization of a molecule–electronics–AI interactive framework in which high-dimensional spectral information is translated into digital cryptographic operations.

The repository contains two independent subprojects:

Repository Structure
ESP32_AS7343_Decoder_Scanner/
│
├── ESP32_AS7343_Decoder_Scanner_trainning/
│
└── ESP32_AS7343_Decoder_Scanner/

1. ESP32_AS7343_Decoder_Scanner_trainning

Purpose:
Collection of spectral fingerprint data for neural network training.

This project streams raw 13-channel spectral data from the AS7343 sensor to Edge Impulse using the Edge Impulse Data Forwarder tool:

https://docs.edgeimpulse.com/tools/clis/edge-impulse-cli/data-forwarder

Function

The firmware:

Acquires full-channel spectral intensity values

Outputs data via Serial interface

Enables structured dataset acquisition under different stimulus states (e.g., encrypted/decrypted)

Training Workflow

Install Edge Impulse CLI.

Run edge-impulse-data-forwarder.

Select the appropriate serial port.

Collect labeled spectral fingerprint samples.

Train classification model in Edge Impulse Studio.

Export Arduino library for deployment.

Consistent sensor configuration (integration time, gain, illumination) must be maintained between training and deployment to ensure model robustness.

2. ESP32_AS7343_Decoder_Scanner

Purpose:
Deployment of trained neural network model for real-time spectral classification and cryptographic decoding.

This firmware integrates:

AS7343 13-channel spectral acquisition

Embedded neural network inference (Edge Impulse export)

QR code scanning via UART

Conditional plaintext/ciphertext output

OLED display

Bluetooth HID keyboard emulation

Operational Logic

Acquire spectral fingerprint.

Perform embedded neural network inference.

Classify system state.

Enable or restrict QR decoding logic based on classification result.

Output decoded information via Serial, OLED, and Bluetooth.

The classifier operates directly in the multi-channel spectral domain rather than compressed RGB space, enabling high-dimensional state discrimination.

Hardware Requirements

ESP32 development board

AS7343 spectral sensor module

UART-based QR code scanner

OLED display (I2C)

USB connection for programming

Default Pin Configuration

AS7343 (I2C)

SDA: GPIO 6

SCL: GPIO 7

QR Scanner (UART2)

RX: GPIO 16

TX: GPIO 17

Modify pin definitions in firmware if necessary.

Conceptual Framework

Conventional optical authentication systems reduce optical information to RGB space (maximum theoretical state space 2^24).
This platform preserves channel-resolved spectral fingerprints acquired from a 13-channel sensor, enabling high-dimensional classification prior to digital execution.

The system implements:

Spectral fingerprint acquisition → AI classification → Conditional cryptographic execution

Thus, decryption becomes contingent on correct spectral state recognition rather than solely digital key validation.

Reproducibility Considerations

Maintain identical sensor gain and integration time across datasets.

Control ambient illumination during both training and deployment.

Ensure Edge Impulse model export version matches firmware implementation.

Recalibration is required if optical path or hardware configuration changes.
