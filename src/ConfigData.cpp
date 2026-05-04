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
        switch (param.type) {
            // Cast the void* pointer back to the original type
            case ParamType::INT:    doc[param.id] = *(int*)param.valuePointer; break;
            case ParamType::FLOAT:  doc[param.id] = *(float*)param.valuePointer; break;
            case ParamType::STRING: doc[param.id] = *(String*)param.valuePointer; break;
            case ParamType::BOOL:   doc[param.id] = *(bool*)param.valuePointer; break;
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
        if (!doc[param.id].isNull()) {
            switch (param.type) {
                case ParamType::INT:    *(int*)param.valuePointer = doc[param.id].as<int>(); break;
                case ParamType::FLOAT:  *(float*)param.valuePointer = doc[param.id].as<float>(); break;
                case ParamType::STRING: *(String*)param.valuePointer = doc[param.id].as<String>(); break;
                case ParamType::BOOL:   *(bool*)param.valuePointer = doc[param.id].as<bool>(); break;
            }
        }
    }
    return true;
}

const std::vector<ConfigParameter>& ConfigData::getParameters() const {
    return _parameters;
}