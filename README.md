# LaserStock

🚀 **LaserStock** is an ESP32-based device that combines a **laser, motion sensor, and servo motor** to visually represent stock prices.

## 📖 Project Overview
This project fetches **Apple (AAPL) stock prices** at boot and every 4 hours, displaying changes through **servo movement** and triggering a **laser** when motion is detected. The laser should be placed on the servo arm, so that you can project the stock price indication on a wall, ceiling or floor. 

## ⚙️ Hardware Components
- **ESP32 D1 Mini**
- **SG90 Micro Servo (connected to IO25)**
- **Laser Module (connected to IO16)**
- **Motion Sensor (connected to IO18)**

## 🛠️ Software Requirements
- **PlatformIO IDE** (VS Code Extension)
- **Arduino Framework for ESP32**

## 📡 API & Network Configuration
Before running, set up your WiFi and API credentials:

1. Copy `include/secrets_template.h` to `include/secrets.h`.
2. Fill in your WiFi SSID, password, and Alpha Vantage API key.

## 🔧 Setup Instructions
1. **Clone the repository:**
   ```sh
   git clone https://github.com/shuuse/LaserStock.git
   cd LaserStock
   ```
2. **Copy and configure secrets:**
   ```sh
   cp include/secrets_template.h include/secrets.h
   nano include/secrets.h
   ```
3. **Open project in PlatformIO (VS Code).**
4. **Compile & Upload** to ESP32.

## 🔍 Features
- **Stock Price Mapping**: Maps AAPL stock price range (240-280) to servo angles (0°-180°).
- **Servo Verification**: Moves from min to max on boot for testing.
- **Motion-Activated Laser**: Lights up for 5 seconds when motion is detected.
- **Smart Stock Updates**: Fetches prices only **during market hours** (9 AM - 4 PM EST).

## 📜 License
MIT License. See `LICENSE` for details.

---
💡 **Contributions & Issues:** Feel free to open an issue or PR to improve the project! 🚀
