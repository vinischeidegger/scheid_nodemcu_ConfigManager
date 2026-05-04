#ifndef CORE_CONFIG_MANAGER_H
#define CORE_CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "ConfigData.h"
#include "NetworkService.h"

/**
 * @brief Options for constructing a ConfigManager instance.
 */
struct ConfigManagerOptions {
    String apSSID = "Scheid Config";
    String mdnsHostname = "configmanager";
    String pageTitle = "Scheid Product Configuration";
};

class ConfigManager {
public:
    /**
     * @brief Constructs a ConfigManager instance with named options.
     * @param options Configuration options for the manager
     */
    ConfigManager(const ConfigManagerOptions& options = ConfigManagerOptions());

    /**
     * @brief Destroys the ConfigManager instance and releases internal resources.
     */
    ~ConfigManager();
    
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

    /**
     * @brief Sets the web portal page title.
     * @param title Title text served by the REST API
     */
    void setPageTitle(const String& title);

    /**
     * @brief Returns the current web portal page title.
     */
    String getPageTitle() const;

    /**
     * @brief Returns the configured mDNS hostname.
     */
    String getMdnsHostname() const;

    /**
     * @brief Returns the configured access point SSID.
     */
    String getApSsid() const;

    /**
     * @brief Returns the configured WiFi SSID.
     */
    String getWifiSsid() const;

    /**
     * @brief Returns the registered configuration parameters.
     */
    const std::vector<ConfigParameter>& getParameters() const;

private:
    ConfigData _configData;
    NetworkService _configService;
    ConfigManagerOptions _options;
};

#endif // CORE_CONFIG_MANAGER_H