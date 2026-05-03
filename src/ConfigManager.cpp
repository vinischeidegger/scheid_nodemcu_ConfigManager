#include "ConfigManager.h"
#include <LittleFS.h>

// Network Libraries (Focused on ESP8266/NodeMCU for this example)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>

// Global instances for Web Server and DNS (kept outside .h to not pollute final products)
ESP8266WebServer server(80);
DNSServer dnsServer;

const byte DNS_PORT = 53;

ConfigManager::ConfigManager(const ConfigManagerOptions& options) {
    // Default factory settings
    _apSSID = options.apSSID;
    _apPassword = ""; // Open Network for Captive Portal
    _wifiSSID = "";
    _wifiPassword = "";
    _mdnsHostname = options.mdnsHostname;
}

void ConfigManager::registerParameter(const String& id, void* valuePointer, ParamType type, const String& label) {
    // Adds the product parameter to our internal list
    _parameters.push_back({id, valuePointer, type, label});
}

bool ConfigManager::begin() {
    Serial.println("[ConfigManager] Initializing...");

    // 1. Starts the file system
    if (!LittleFS.begin()) {
        Serial.println("[ConfigManager] Failed to mount LittleFS. Formatting...");
        LittleFS.format(); // Attempts to format on first use
        LittleFS.begin();
    }

    // 2. Loads configuration (if it exists) to registered variables
    loadConfig();

    // 3. Attempts to connect to WiFi
    if (_wifiSSID != "" && connectWiFi()) {
        Serial.println("[ConfigManager] Connected to WiFi successfully!");
        return true;
    }

    // 4. If it failed or WiFi is not configured, raise the Captive Portal
    Serial.println("[ConfigManager] Starting AP mode and Captive Portal...");
    startAP();
    setupCaptivePortal();
    return false;
}

void ConfigManager::handle() {
    // Keeps the Web server and DNS running in the background
    dnsServer.processNextRequest();
    server.handleClient();
}

bool ConfigManager::saveConfig() {
    Serial.println("[ConfigManager] Saving configuration...");

    // Using ArduinoJson v7 (dynamic and optimized JsonDocument)
    JsonDocument doc; 

    // Saves network credentials
    doc["wifi_ssid"] = _wifiSSID;
    doc["wifi_pass"] = _wifiPassword;

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

bool ConfigManager::loadConfig() {
    Serial.println("[ConfigManager] Loading configuration...");

    File file = LittleFS.open(_configPath, "r");
    if (!file) return false; // File does not exist yet

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("[ConfigManager] Failed to read JSON.");
        return false;
    }

    // Loads network credentials
    if (doc["wifi_ssid"].is<String>()) 
        _wifiSSID = doc["wifi_ssid"].as<String>();

    if (doc["wifi_pass"].is<String>()) 
        _wifiPassword = doc["wifi_pass"].as<String>();

    // Dynamically updates variables in the product's main.cpp
    for (const auto& param : _parameters) {
        if (!doc["key"].isNull()) {
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

// ==========================================
// NETWORK METHODS (Basic Implementation)
// ==========================================

bool ConfigManager::connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifiSSID.c_str(), _wifiPassword.c_str());
    
    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) { // Timeout of ~10 seconds
        delay(500);
        Serial.print(".");
        tentativas++;
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

void ConfigManager::startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_apSSID.c_str(), _apPassword.c_str());
    
    // Configures DNS to redirect ALL traffic to the NodeMCU IP (Captive Portal)
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    
    // Starts mDNS for access via friendly hostname (e.g., http://configmanager.local)
    if (!MDNS.begin(_mdnsHostname.c_str())) {
        Serial.println("[ConfigManager] Error starting mDNS");
    } else {
        Serial.print("[ConfigManager] mDNS started: http://");
        Serial.print(_mdnsHostname);
        Serial.println(".local");
    }
}

void ConfigManager::setupCaptivePortal() {
    // Allows mDNS requests
    MDNS.addService("http", "tcp", 80);
    
    // Root route (where we will build the HTML)
    server.on("/", []() {
        server.send(200, "text/html", "<h1>Welcome to Configuration</h1><p>Access via: <b>http://configmanager.local</b></p>");
    });

    // Route to get form data when user clicks "Save"
    server.on("/salvar", HTTP_POST, [this]() {
        // Logic to get POST data, update _parameters and call saveConfig()
        server.send(200, "text/html", "<h1>Saved! Restarting...</h1>");
        delay(2000);
        ESP.restart();
    });

    // Catch-all route to force Captive Portal on mobile devices
    server.onNotFound([]() {
        server.sendHeader("Location", "/", true); // Redirects to root
        server.send(302, "text/plain", "");
    });

    server.begin();
}

void ConfigManager::setMdnsHostname(const String& hostname) {
    _mdnsHostname = hostname;
    Serial.print("[ConfigManager] mDNS hostname set to: ");
    Serial.println(hostname);
}