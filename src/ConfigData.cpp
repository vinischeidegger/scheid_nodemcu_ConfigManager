#include "ConfigData.h"
#include <LittleFS.h>
#include <limits.h>
#include <math.h>

static bool ensureConfigFilesystemReady() {
    if (LittleFS.begin()) {
        return true;
    }

    Serial.println("[ConfigData] Failed to mount LittleFS.");
    return false;
}

static bool isConfigParameterIdRegistered(const std::vector<ConfigParameter>& parameters, const String& id) {
    for (const auto& parameter : parameters) {
        if (parameter.id == id) {
            return true;
        }
    }
    return false;
}

static bool readInt32Param(const JsonVariantConst& valueNode, int32_t& outputValue) {
    if (!valueNode.is<long long>() && !valueNode.is<unsigned long long>()) {
        return false;
    }

    long long rawValue = valueNode.as<long long>();
    if (rawValue < INT32_MIN || rawValue > INT32_MAX) {
        return false;
    }

    outputValue = static_cast<int32_t>(rawValue);
    return true;
}

static bool readUInt32Param(const JsonVariantConst& valueNode, uint32_t& outputValue) {
    if (valueNode.is<long long>()) {
        long long signedValue = valueNode.as<long long>();
        if (signedValue < 0) {
            return false;
        }

        outputValue = static_cast<uint32_t>(signedValue);
        return true;
    }

    if (valueNode.is<unsigned long long>()) {
        unsigned long long rawValue = valueNode.as<unsigned long long>();
        if (rawValue > UINT32_MAX) {
            return false;
        }

        outputValue = static_cast<uint32_t>(rawValue);
        return true;
    }

    return false;
}

static bool readFloatParam(const JsonVariantConst& valueNode, float& outputValue) {
    if (!valueNode.is<float>() && !valueNode.is<double>() && !valueNode.is<long long>() && !valueNode.is<unsigned long long>()) {
        return false;
    }

    float value = valueNode.as<float>();
    if (!isfinite(value)) {
        return false;
    }

    outputValue = value;
    return true;
}

static bool readDoubleParam(const JsonVariantConst& valueNode, double& outputValue) {
    if (!valueNode.is<float>() && !valueNode.is<double>() && !valueNode.is<long long>() && !valueNode.is<unsigned long long>()) {
        return false;
    }

    double value = valueNode.as<double>();
    if (!isfinite(value)) {
        return false;
    }

    outputValue = value;
    return true;
}

void ConfigData::registerParameter(const String& id, void* valuePointer, ParamType type, const String& label) {
    if (id.isEmpty()) {
        Serial.println("[ConfigData] Ignoring parameter registration with empty id.");
        return;
    }

    if (valuePointer == nullptr) {
        Serial.print("[ConfigData] Ignoring parameter '");
        Serial.print(id);
        Serial.println("' because pointer is null.");
        return;
    }

    if (isConfigParameterIdRegistered(_parameters, id)) {
        Serial.print("[ConfigData] Ignoring duplicate parameter id: ");
        Serial.println(id);
        return;
    }

    // Adds the product parameter to our internal list
    _parameters.push_back({id, valuePointer, type, label});
}

void ConfigData::registerParameter(const String& id, int& value, const String& label) {
    registerParameter(id, &value, ParamType::INT, label);
}

void ConfigData::registerParameter(const String& id, uint32_t& value, const String& label) {
    registerParameter(id, &value, ParamType::UINT32, label);
}

void ConfigData::registerParameter(const String& id, unsigned long& value, const String& label) {
    registerParameter(id, &value, ParamType::ULONG, label);
}

void ConfigData::registerParameter(const String& id, float& value, const String& label) {
    registerParameter(id, &value, ParamType::FLOAT, label);
}

void ConfigData::registerParameter(const String& id, double& value, const String& label) {
    registerParameter(id, &value, ParamType::DOUBLE, label);
}

void ConfigData::registerParameter(const String& id, bool& value, const String& label) {
    registerParameter(id, &value, ParamType::BOOL, label);
}

void ConfigData::registerParameter(const String& id, String& value, const String& label) {
    registerParameter(id, &value, ParamType::STRING, label);
}

bool ConfigData::saveConfig() {
    Serial.println("[ConfigData] Saving configuration...");

    if (!ensureConfigFilesystemReady()) {
        return false;
    }

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
            case ParamType::UINT32: {
                uint32_t* value = static_cast<uint32_t*>(param.valuePointer);
                doc[param.id] = *value;
                break;
            }
            case ParamType::ULONG: {
                unsigned long* value = static_cast<unsigned long*>(param.valuePointer);
                doc[param.id] = *value;
                break;
            }
            case ParamType::FLOAT: {
                float* value = static_cast<float*>(param.valuePointer);
                doc[param.id] = *value;
                break;
            }
            case ParamType::DOUBLE: {
                double* value = static_cast<double*>(param.valuePointer);
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

    doc["wifi_ssid"] = _wifiSsid;
    doc["wifi_password"] = _wifiPassword;

    // Opens the file and writes
    File file = LittleFS.open(_configPath, "w");
    if (!file) return false;

    serializeJson(doc, file);
    file.close();
    return true;
}

bool ConfigData::loadConfig() {
    Serial.println("[ConfigData] Loading configuration...");

    if (!ensureConfigFilesystemReady()) {
        return false;
    }

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

        JsonVariantConst valueNode = doc[param.id];

        switch (param.type) {
            case ParamType::INT: {
                int* value = static_cast<int*>(param.valuePointer);
                int32_t parsedValue = 0;
                if (readInt32Param(valueNode, parsedValue)) {
                    *value = static_cast<int>(parsedValue);
                }
                break;
            }
            case ParamType::UINT32: {
                uint32_t* value = static_cast<uint32_t*>(param.valuePointer);
                uint32_t parsedValue = 0;
                if (readUInt32Param(valueNode, parsedValue)) {
                    *value = parsedValue;
                }
                break;
            }
            case ParamType::ULONG: {
                unsigned long* value = static_cast<unsigned long*>(param.valuePointer);
                uint32_t parsedValue = 0;
                if (readUInt32Param(valueNode, parsedValue)) {
                    *value = static_cast<unsigned long>(parsedValue);
                }
                break;
            }
            case ParamType::FLOAT: {
                float* value = static_cast<float*>(param.valuePointer);
                float parsedValue = 0.0f;
                if (readFloatParam(valueNode, parsedValue)) {
                    *value = parsedValue;
                }
                break;
            }
            case ParamType::DOUBLE: {
                double* value = static_cast<double*>(param.valuePointer);
                double parsedValue = 0.0;
                if (readDoubleParam(valueNode, parsedValue)) {
                    *value = parsedValue;
                }
                break;
            }
            case ParamType::STRING: {
                String* value = static_cast<String*>(param.valuePointer);
                if (valueNode.is<const char*>()) {
                    *value = valueNode.as<String>();
                }
                break;
            }
            case ParamType::BOOL: {
                bool* value = static_cast<bool*>(param.valuePointer);
                if (valueNode.is<bool>()) {
                    *value = valueNode.as<bool>();
                }
                break;
            }
        }
    }

    if (!doc["wifi_ssid"].isNull()) {
        _wifiSsid = doc["wifi_ssid"].as<String>();
    }
    if (!doc["wifi_password"].isNull()) {
        _wifiPassword = doc["wifi_password"].as<String>();
    }

    Serial.print("[ConfigData] Loaded WiFi SSID: ");
    Serial.println(_wifiSsid.isEmpty() ? "<empty>" : _wifiSsid);
    return true;
}

void ConfigData::setWifiCredentials(const String& ssid, const String& password) {
    _wifiSsid = ssid;
    _wifiPassword = password;
}

String ConfigData::getWifiSsid() const {
    return _wifiSsid;
}

String ConfigData::getWifiPassword() const {
    return _wifiPassword;
}

const std::vector<ConfigParameter>& ConfigData::getParameters() const {
    return _parameters;
}