#include "ConfigData.h"
#include <LittleFS.h>

void ConfigData::registerParameter(const String& id, void* valuePointer, ParamType type, const String& label) {
    // Adds the product parameter to our internal list
    _parameters.push_back({id, valuePointer, type, label});
}

bool ConfigData::saveConfig() {
    Serial.println("[ConfigData] Saving configuration...");

    // Using ArduinoJson v7 (dynamic and optimized JsonDocument)
    JsonDocument doc;

    // Dynamically saves ALL parameters registered by products
    for (const auto& param : _parameters) {
        if (!param.valuePointer) {
            continue;
        }

        switch (param.type) {
            case ParamType::INT: {
                int* value = static_cast<int*>(param.valuePointer);
                doc[param.id] = *value;
                break;
            }
            case ParamType::FLOAT: {
                float* value = static_cast<float*>(param.valuePointer);
                doc[param.id] = *value;
                break;
            }
            case ParamType::STRING: {
                String* value = static_cast<String*>(param.valuePointer);
                doc[param.id] = *value;
                break;
            }
            case ParamType::BOOL: {
                bool* value = static_cast<bool*>(param.valuePointer);
                doc[param.id] = *value;
                break;
            }
        }
    }

    // Opens the file and writes
    File file = LittleFS.open(_configPath, "w");
    if (!file) return false;

    serializeJson(doc, file);
    file.close();
    return true;
}

bool ConfigData::loadConfig() {
    Serial.println("[ConfigData] Loading configuration...");

    File file = LittleFS.open(_configPath, "r");
    if (!file) return false; // File does not exist yet

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("[ConfigData] Failed to read JSON.");
        return false;
    }

    // Dynamically updates variables in the product's main.cpp
    for (const auto& param : _parameters) {
        if (!param.valuePointer || doc[param.id].isNull()) {
            continue;
        }

        switch (param.type) {
            case ParamType::INT: {
                int* value = static_cast<int*>(param.valuePointer);
                *value = doc[param.id].as<int>();
                break;
            }
            case ParamType::FLOAT: {
                float* value = static_cast<float*>(param.valuePointer);
                *value = doc[param.id].as<float>();
                break;
            }
            case ParamType::STRING: {
                String* value = static_cast<String*>(param.valuePointer);
                *value = doc[param.id].as<String>();
                break;
            }
            case ParamType::BOOL: {
                bool* value = static_cast<bool*>(param.valuePointer);
                *value = doc[param.id].as<bool>();
                break;
            }
        }
    }
    return true;
}

const std::vector<ConfigParameter>& ConfigData::getParameters() const {
    return _parameters;
}