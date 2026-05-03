#ifndef CORE_CONFIG_MANAGER_H
#define CORE_CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>

/**
 * @brief Supported data types for configuration parameters.
 */
enum class ParamType {
    INT,
    FLOAT,
    STRING,
    BOOL
};

/**
 * @brief Internal structure to map product variables to ConfigManager.
 */
struct ConfigParameter {
    String id;
    void* valuePointer;
    ParamType type;
    String label;
};

class ConfigManager {
public:
    ConfigManager();
    
    /**
     * @brief Registers a parameter that will be managed by the portal and saved in memory.
     * @param id Unique identifier in JSON (e.g., "setpoint_pressao")
     * @param valuePointer Pointer to the variable in your main code
     * @param type Data type (ParamType)
     * @param label Friendly name that will appear on the mobile HTML form
     */
    void registerParameter(const String& id, void* valuePointer, ParamType type, const String& label);

    /**
     * @brief Initializes the system, mounts the file system, and attempts to load configuration.
     * If unable to connect to saved WiFi, automatically enters Access Point mode.
     */
    bool begin();

    /**
     * @brief Must be called within the main loop() to keep the WebServer and DNS active.
     */
    void handle();

    /**
     * @brief Saves the current values of registered variables to the file system (LittleFS).
     */
    bool saveConfig();

    /**
     * @brief Loads the values from the JSON file to the registered variables.
     */
    bool loadConfig();

    /**
     * @brief Resets network configuration (clears WiFi and enters AP mode).
     */
    void resetNetwork();

    /**
     * @brief Sets the mDNS hostname for friendly access (e.g., "configmanager.local").
     * @param hostname Name without ".local" (e.g., "configure" will result in "configure.local")
     */
    void setMdnsHostname(const String& hostname);

private:
    std::vector<ConfigParameter> _parameters;
    String _apSSID;
    String _apPassword;
    
    // Internal Network Configuration
    String _wifiSSID;
    String _wifiPassword;
    String _mdnsHostname = "configmanager";

    // Private Methods
    void setupCaptivePortal();
    void startAP();
    bool connectWiFi();
    
    // Path Constants
    const char* _configPath = "/config.json";
};

#endif // CORE_CONFIG_MANAGER_H