#ifndef MOCK_ARDUINO_JSON_H
#define MOCK_ARDUINO_JSON_H

#include "mock_arduino.h"
#include <map>

namespace LittleFS {
    class File;
}

namespace ArduinoJson {
    class JsonDocument {
    public:
        class JsonVariant {
        public:
            JsonVariant(std::string& storage) : _storage(storage) {}
            JsonVariant(const std::string& value) : _value(value), _storage(_value) {}

            template<typename T>
            JsonVariant& operator=(const T& value) {
                _storage = std::to_string(value);
                return *this;
            }

            JsonVariant& operator=(const String& value) {
                _storage = value.c_str();
                return *this;
            }

            bool isNull() const { return _storage.empty(); }

            template<typename T>
            T as() const;

        private:
            std::string _value;
            std::string& _storage;
            std::string* _valueRef = nullptr;
        };

        JsonDocument() = default;

        JsonVariant operator[](const char* key) {
            return JsonVariant(_storage[key]);
        }

        JsonVariant operator[](const char* key) const {
            auto it = _storage.find(key);
            if (it == _storage.end()) {
                return JsonVariant("");
            }
            return JsonVariant(it->second);
        }

    private:
        std::map<std::string, std::string> _storage;
    };

    template<>
    inline int JsonDocument::JsonVariant::as<int>() const {
        return _storage.empty() ? 0 : std::stoi(_storage);
    }

    template<>
    inline float JsonDocument::JsonVariant::as<float>() const {
        return _storage.empty() ? 0.0f : std::stof(_storage);
    }

    template<>
    inline String JsonDocument::JsonVariant::as<String>() const {
        return String(_storage.c_str());
    }

    template<>
    inline bool JsonDocument::JsonVariant::as<bool>() const {
        return _storage == "1" || _storage == "true";
    }

    template<typename T>
    class DeserializationError {
    public:
        DeserializationError(bool ok = true) : _ok(ok) {}
        operator bool() const { return !_ok; }
    private:
        bool _ok;
    };

    template<typename T>
    DeserializationError<T> deserializeJson(T& doc, const std::string& json) {
        (void)doc;
        (void)json;
        return DeserializationError<T>(true);
    }

    template<typename T>
    void serializeJson(const T& doc, std::string& output) {
        (void)doc;
        output = "{}";
    }

    template<typename T>
    DeserializationError<T> deserializeJson(T& doc, const LittleFS::File& file) {
        (void)doc;
        (void)file;
        return DeserializationError<T>(true);
    }

    template<typename T>
    void serializeJson(const T& doc, LittleFS::File& file) {
        (void)doc;
        (void)file;
    }
}

#endif // MOCK_ARDUINO_JSON_H
