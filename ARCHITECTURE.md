# Project Architecture & Copilot Instructions

## Role
You are an expert C++ embedded systems engineer. You are assisting in building the `Core_ConfigManager` library using PlatformIO.

## Tech Stack
* **Environment:** PlatformIO (VS Code)
* **Framework:** Arduino (C++)
* **Target Hardware:** ESP8266 (NodeMCU) and ESP32
* **Dependencies:** `ArduinoJson` (for storage), `ESP8266WiFi` / `DNSServer` / `ESPAsyncWebServer` (or standard `WebServer`) for Captive Portal.

## Architecture Rules
1.  **Library Structure:** This is a standalone, private PlatformIO library. It is NOT a monolithic product application. It will be imported into multiple external product repositories via GitHub URLs.
2.  **Responsibility:** This module handles WiFi connection, Captive Portal fallback (AP mode), and persistent storage (LittleFS/SPIFFS) via JSON.
3.  **Dependency Injection:** Do not hardcode product-specific variables. Use a registration system (e.g., `registerParameter()`) so external apps can inject their specific variables (like pressure setpoints or flow limits) into this manager.
4.  **Memory Optimization:** Use pointers, references, and avoid String duplication where possible. ESP hardware has limited RAM.
5.  **Testing:** Code must be testable. Isolate logic for Native testing (`gtest`) and hardware testing (`Unity`).

## Git Convention
* **Branching:** GitHub Flow (feature branches merging into `master`).
* **Commits:** Strictly use Conventional Commits (`feat:`, `fix:`, `chore:`, `refactor:`).