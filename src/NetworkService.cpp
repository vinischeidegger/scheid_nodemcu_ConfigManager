#include "NetworkService.h"
#include <LittleFS.h>

// Network Libraries (Focused on ESP8266/NodeMCU for this example)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

NetworkService::NetworkService(ConfigData& configData)
    : _configData(configData) {
    // Allocate portal and API helpers after this object exists.
    _captivePortal = new CaptivePortal(*this);
    _restApi = new RestApi(*this, _captivePortal->getServer());
}

NetworkService::~NetworkService() {
    delete _restApi;
    delete _captivePortal;
}

bool NetworkService::begin(const String& apSSID, const String& wifiSSID, const String& wifiPassword, const String& mdnsHostname) {
    Serial.println("[NetworkService] Initializing...");

    _apSSID = apSSID;
    _wifiSSID = wifiSSID;
    _wifiPassword = wifiPassword;
    _mdnsHostname = mdnsHostname;

    // 1. Starts the file system
    if (!LittleFS.begin()) {
        Serial.println("[NetworkService] Failed to mount LittleFS. Formatting...");
        LittleFS.format(); // Attempts to format on first use
        LittleFS.begin();
    }

    // 2. Attempts to connect to WiFi (config already loaded by ConfigManager)
    if (_wifiSSID != "" && connectWiFi()) {
        Serial.println("[NetworkService] Connected to WiFi successfully!");
        return true;
    }

    // 3. If it failed or WiFi is not configured, raise the Captive Portal
    Serial.println("[NetworkService] Starting AP mode and Captive Portal...");
    _captivePortal->setup(_apSSID, _mdnsHostname);
    _restApi->setup();
    return false;
}

void NetworkService::handle() {
    // Keeps the Captive Portal running in the background
    _captivePortal->handle();
}

void NetworkService::setMdnsHostname(const String& hostname) {
    _mdnsHostname = hostname;
    Serial.print("[NetworkService] mDNS hostname set to: ");
    Serial.println(hostname);
}

String NetworkService::getMdnsHostname() const {
    return _mdnsHostname;
}

String NetworkService::getApSsid() const {
    return _apSSID;
}

String NetworkService::getWifiSsid() const {
    return _wifiSSID;
}

String NetworkService::getPageTitle() const {
    return _pageTitle;
}

void NetworkService::setPageTitle(const String& title) {
    _pageTitle = title;
}

const std::vector<ConfigParameter>& NetworkService::getParameters() const {
    return _configData.getParameters();
}

bool NetworkService::connectWiFi() {
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