# ESP32-S3-DEVKITC-1-N32R8V Board Test Project-V1.0

This repository contains a simple test program to jump-start development on the ESP32-S3-DEVKITC-1-N32R8V board, which features an ESP32-S3 WROOM-2 module with 32 MB Octal Flash and 8 MB Octal RAM.

![image](https://github.com/nzwildcode/ESP32-S3-DEVKITC-1-N32R8V-Board-Tester/assets/31437739/5980596e-ce0d-46d7-b74a-c25e3db13a7a)
## Features

- Configurable relay timers using an AsyncWebServer
- Real-time updating of relay states and timer values
- Utilization of LittleFS for persistent data storage
- OTA (Over-The-Air) firmware updates using ArduinoOTA
- NeoPixel LED status indicator
- Relay control
- Status LED

## Hardware Components

### NeoPixel LED (GPIO 38)

- Green: Timer running, relay on
- Red: Timer running, relay off
- Off: Timer not running

### Relay (GPIO 15)

Controlled based on timer values:
- On: During set on-time
- Off: During set off-time

### Status LED (GPIO 13)

Indicates relay state:
- On: Relay active
- Off: Relay inactive

## Getting Started

1. Obtain the project files:
   - Clone this repository, or
   - Download as a ZIP file and extract it

2. Open the project in VS Code using PlatformIO

3. Build the firmware:
   - Click the checkmark icon (✓) in the bottom toolbar, or
   - Run: `pio run`

4. Upload the firmware:
   - Connect your ESP32 board via USB
   - Click the arrow icon (→) in the bottom toolbar, or
   - Run: `pio run --target upload`

5. Upload the file system image:
   - **Important:** Close the Serial Monitor if it's open
   - Click "Upload Filesystem Image" in Project Tasks, or
   - Run: `pio run --target uploadfs`

6. Debug (optional):
   - Open Serial Monitor (plug icon 🔌) with baud rate 115200

7. Connect to the ESP32's Wi-Fi hotspot (default: "ESP32 Access Point")

8. Access the web interface: Navigate to `http://192.168.4.1` in a web browser

**Note:** If you encounter issues, ensure you have the latest board definitions and libraries installed. Try `pio update` to update all libraries.

**Note:** mDNS is not implemented. Connect to the board's hotspot and use the IP address `192.168.4.1` to access the web interface.

## Usage

1. Access the web interface at `192.168.4.1`.
2. Configure relay timers and other settings as needed.
3. Monitor real-time updates of relay states and timer values.

## Contributing

Contributions are welcome! Please open a pull request with your proposed changes or improvements.

## License

This project is licensed under the MIT License.  

## Acknowledgements

Gratitude to Boris Jäger (sivar2311) from the PlatformIO community for guidance with the correct platform.ini code.
