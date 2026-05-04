# Copilot Onboarding Instructions

## Repository summary

- This repository is a PlatformIO Arduino C++ library intended for ESP8266 and ESP32.
- It provides WiFi connection management, captive portal fallback (Access Point mode), and JSON-based persistent storage on LittleFS/SPIFFS.
- The library is designed as reusable firmware infrastructure, not as a product application.

## Observed repository contents

- Current checkout contains only:
  - `README.md`
  - `ARCHITECTURE.md`
  - `.github/copilot-instructions.md`
- No source or PlatformIO configuration files are present in this workspace snapshot.
- The README describes expected directories such as `include/`, `src/`, `test/`, `examples/`, and files such as `platformio.ini` and `library.json`, but they are not visible here.

## Build and validation guidance

- The primary validation mechanism is PlatformIO. When source/config files are available, use:
  - `pio test -e native` — run native Google Test logic tests.
  - `pio test -e nodemcuv2` — run embedded Unity/ESP8266 tests when hardware is attached.
- Always run commands from the repository root.
- If `pio` is not installed in the environment, install PlatformIO CLI before attempting build/test.
- If the repository does not contain `platformio.ini` or any code directories, do not attempt a PlatformIO build; instead confirm the correct checkout.
- Use `git status --short` to validate local file state after making edits.
- If a `platformio.ini` file appears, inspect its environments and test targets before choosing commands.
- **Important**: Only run `pio run` on example folders (e.g., `examples/BasicSetup/`). Do not run `pio run` on the main library folder as it lacks the required `setup()` and `loop()` functions for a complete Arduino sketch.

## Project layout expectations

- Major project elements described in the README:
  - `include/` — public header files.
  - `src/` — implementation `.cpp` files.
  - `examples/` — sample sketches.
  - `test/` — unit tests for native and embedded flows.
  - `library.json` — PlatformIO library metadata and dependency declarations.
  - `platformio.ini` — development and test environment configuration.
- Key documentation/configuration files:
  - `README.md`
  - `ARCHITECTURE.md`
  - `.github/copilot-instructions.md`
- No GitHub Actions workflows are currently present in this checkout.

## Validation rules for the agent

- Treat this repository as a generic library module. Do not add product-specific device logic or app-specific naming.
- Respect the README and architecture notes as the authoritative architecture description.
- If source files are missing, do not invent the implementation. Instead document that code is absent and request the correct repository state.
- Prefer non-breaking, minimal changes that keep the library reusable across external product repositories.
- Use this instructions file first; only search the repository if the information here is incomplete or if new files appear after a checkout update.

## Best practices for working here

- Validate changes by running the test commands when the code and PlatformIO files exist.
- Keep modifications consistent with PlatformIO/Arduino library expectations.
- Preserve the library's responsibility: WiFi/captive-portal handling and persistent JSON configuration storage for ESP devices.
- If no build artifacts or config files exist, do not assume a build pipeline exists in this checkout.

## Code documentation requirements

- **All code documentation must be in English.** This includes:
  - Function/method documentation and comments
  - Variable and constant descriptions
  - Inline code explanations
  - Class and namespace documentation
  - README files, ARCHITECTURE documents, and all guides
- Use clear, precise English without abbreviations unless standard in the industry.
- Ensure documentation is comprehensive enough for external developers to understand the code.
