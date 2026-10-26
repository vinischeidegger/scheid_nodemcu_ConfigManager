#ifndef CORE_CONFIG_DATA_H
#define CORE_CONFIG_DATA_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdint.h>
#include <vector>

/**
 * @brief Supported data types for configuration parameters.
 */
enum class ParamType {
    INT,
    UINT32,
    ULONG,
    FLOAT,
    DOUBLE,
    STRING,
    BOOL
};

/**
 * @brief Internal structure to map product variables to ConfigData.
 */
struct ConfigParameter {
    String id;
    void* valuePointer;
    ParamType type;
    String label;
};

class ConfigData {
public:
    /**
     * @brief Registers a parameter that will be managed and saved in memory.
     * @param id Unique identifier in JSON (e.g., "setpoint_pressao")
     * @param valuePointer Pointer to the variable in your main code
     * @param type Data type (ParamType)
     * @param label Friendly name that will appear on the mobile HTML form
     */
    void registerParameter(const String& id, void* valuePointer, ParamType type, const String& label);

    /**
     * @brief Type-safe overloads for parameter registration.
     */
    void registerParameter(const String& id, int& value, const String& label);
    void registerParameter(const String& id, uint32_t& value, const String& label);
    void registerParameter(const String& id, unsigned long& value, const String& label);
    void registerParameter(const String& id, float& value, const String& label);
    void registerParameter(const String& id, double& value, const String& label);
    void registerParameter(const String& id, bool& value, const String& label);
    void registerParameter(const String& id, String& value, const String& label);

    /**
     * @brief Saves the current values of registered variables to the file system (LittleFS).
     */
    bool saveConfig();

    /**
     * @brief Loads the values from the JSON file to the registered variables.
     */
    bool loadConfig();

    /**
     * @brief Stores WiFi credentials in memory so they can be persisted with the next save.
     */
    void setWifiCredentials(const String& ssid, const String& password);

    /**
     * @brief Returns the stored WiFi SSID, if any.
     */
    String getWifiSsid() const;

    /**
     * @brief Returns the stored WiFi password, if any.
     */
    String getWifiPassword() const;

    /**
     * @brief Returns the registered configuration parameters.
     */
    const std::vector<ConfigParameter>& getParameters() const;

    /**
        * @brief Applies values from a JSON map and updates registered parameters in RAM.
        * @param flatValues JSON object with parameter values
     * @return True if at least one parameter was successfully updated
     */
        bool applyParameterValues(const JsonVariantConst& flatValues);

private:
    std::vector<ConfigParameter> _parameters;
    String _wifiSsid;
    String _wifiPassword;

    // Path Constants
    const char* _configPath = "/config.json";
};

#endif // CORE_CONFIG_DATA_H