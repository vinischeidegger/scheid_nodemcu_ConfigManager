# ConfigManager ⚙️

![Platform](https://img.shields.io/badge/Platform-PlatformIO-orange.svg)
![Framework](https://img.shields.io/badge/Framework-Arduino-blue.svg)
![Hardware](https://img.shields.io/badge/Supported_Hardware-ESP8266%20%7C%20ESP32-lightgrey.svg)

**ConfigManager** is a robust, reusable C++ library designed to handle WiFi connectivity, Captive Portal configuration, and persistent parameter storage for IoT devices. 

It abstracts away the boilerplate of network setup and LittleFS/SPIFFS file management, allowing developers to focus entirely on the product's business logic.

---

## ✨ Features

* **Captive Portal:** Automatically falls back to Access Point (AP) mode if WiFi connection fails, spinning up a mobile-friendly configuration web page.
* **Dynamic Parameter Injection:** Easily register product-specific variables (e.g., pressure setpoints, flow limits). The Captive Portal dynamically generates form fields for them.
* **Persistent Storage:** Saves and loads configurations reliably using JSON over LittleFS/SPIFFS.
* **Modular and Reusable:** Built with Dependency Injection in mind, using clean interfaces for hardware and network services.
* **Memory Optimized:** Designed specifically for ESP8266/NodeMCU and ESP32 with strict memory constraints.

---

## 📦 Installation

This module is designed to be used as a private library in a multi-repo architecture. Do not clone this repository directly into your workspace. Instead, add it to your product's `platformio.ini` file:

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino

lib_deps =
    ; Import the library directly via GitHub URL (use tags to lock versions)
    [https://github.com/YourOrganization/ConfigManager.git#v1.0.0](https://github.com/YourOrganization/ConfigManager.git#v1.0.0)
    
    ; Dependencies required by this module
    bblanchon/ArduinoJson @ ^6.21.0
```

## 📂 Deployment & Filesystem

> [!IMPORTANT]
> **Don't forget the Filesystem!**
> This library uses **LittleFS** to serve the web interface. Uploading the firmware (using the standard **Upload** button) **does not** upload the web files.

### How to Upload the Web Interface
To ensure the configuration page works, you must run the **Upload Filesystem Image** task in PlatformIO:

1.  Open the **PlatformIO** sidebar (the Ant head icon).
2.  Navigate to your Project's environment (e.g., `env:nodemcuv2`).
3.  Expand the **Platform** folder.
4.  Click **Upload Filesystem Image**.

### Asset Automation
This library includes a custom build script that handles everything for you during the build process:
*   **Sync**: Automatically copies library assets into your `data/cm` folder.
*   **Compression**: Gzips your HTML, CSS, and JS files to save space on the NodeMCU.
*   **Isolation**: Keeps library files in the `/cm` subdirectory to avoid overwriting your own project files.

**Note:** You do not need to manually copy any files. Just run the upload task above and the script will ensure your device has the latest, most optimized web assets.

## 🚀 Quick Start

Here is a minimal example of how to implement `ConfigManager` in a new product. (See `src/main.cpp` for a complete working example.)

```cpp
#include <Arduino.h>
#include "ConfigManager.h"

// 1. Instantiate the manager
ConfigManager configManager;

// 2. Define product-specific variables
float pressureSetpoint = 15.5;
String deviceName = "PressureSensor_01";

void setup() {
    Serial.begin(115200);

    // 3. Register your custom parameters so the Captive Portal knows about them
    // (Key, Pointer to variable, Data Type)
    configManager.registerParameter("pressure_sp", &pressureSetpoint, "float");
    configManager.registerParameter("device_name", &deviceName, "string");

    // 4. Initialize the manager. 
    // It will attempt to load saved configs and connect to WiFi. 
    // If it fails, it starts the Captive Portal AP.
    configManager.begin();
}

void loop() {
    // Keeps the Captive Portal web server responsive if in AP mode
    configManager.handle();

    // Your main product logic goes here...
}
```

---

## 📂 Repository Structure

```text
ConfigManager/
├── include/                  # Header files (.h)
├── src/                      # Implementation files (.cpp) and sample main.cpp
├── examples/                 # Additional ready-to-compile examples for standard use cases
├── library.json              # PlatformIO library manifest and dependencies
└── platformio.ini            # PlatformIO project configuration
```

---

## 🤝 Contributing

When making changes to this core library, ensure that you:
1.  Do not introduce product-specific logic (e.g., no code specifically for the Flow Monitor). Keep it generic!
3.  Update the version tag in `library.json` following [Semantic Versioning](https://semver.org/).