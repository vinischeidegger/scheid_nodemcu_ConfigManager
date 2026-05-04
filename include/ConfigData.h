#ifndef CORE_CONFIG_DATA_H
#define CORE_CONFIG_DATA_H

#include <Arduino.h>
#include <ArduinoJson.h>
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
     * @brief Saves the current values of registered variables to the file system (LittleFS).
     */
    bool saveConfig();

    /**
     * @brief Loads the values from the JSON file to the registered variables.
     */
    bool loadConfig();

    /**
     * @brief Returns the registered configuration parameters.
     */
    const std::vector<ConfigParameter>& getParameters() const;

private:
    std::vector<ConfigParameter> _parameters;

    // Path Constants
    const char* _configPath = "/config.json";
};

#endif // CORE_CONFIG_DATA_H