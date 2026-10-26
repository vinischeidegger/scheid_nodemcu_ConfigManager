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
    if (_wifiSSID != "" && connectToWiFi(_wifiSSID, _wifiPassword, false)) {
        _captivePortal->setupStationMode(_mdnsHostname);
        _restApi->setup();
        _captivePortal->getServer().begin();
        Serial.println("[NetworkService] Connected to WiFi successfully!");
        Serial.print("[NetworkService] STA SSID: ");
        Serial.println(_wifiSSID);
        Serial.print("[NetworkService] STA IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    // 3. If it failed or WiFi is not configured, raise the Captive Portal
    Serial.println("[NetworkService] Starting AP mode and Captive Portal...");
    _captivePortal->setup(_apSSID, _mdnsHostname);
    _restApi->setup();
    Serial.println("[NetworkService] REST endpoints setup complete");
    // Start the web server after REST endpoints are registered so /api/* handlers are active
    _captivePortal->getServer().begin();
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

String NetworkService::getWifiMode() const {
    WiFiMode_t mode = WiFi.getMode();
    if (mode == WIFI_STA) {
        return "STA";
    }
    if (mode == WIFI_AP) {
        return "AP";
    }
    if (mode == WIFI_AP_STA) {
        return "AP_STA";
    }
    return "OFF";
}

String NetworkService::getPageTitle() const {
    return _pageTitle;
}

void NetworkService::setPageTitle(const String& title) {
    _pageTitle = title;
}

bool NetworkService::connectToWiFi(const String& wifiSSID, const String& wifiPassword, bool keepAccessPoint) {
    _wifiSSID = wifiSSID;
    _wifiPassword = wifiPassword;

    WiFi.mode(keepAccessPoint ? WIFI_AP_STA : WIFI_STA);
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

bool NetworkService::saveWifiCredentials(const String& wifiSSID, const String& wifiPassword) {
    _wifiSSID = wifiSSID;
    _wifiPassword = wifiPassword;
    _configData.setWifiCredentials(wifiSSID, wifiPassword);
    return _configData.saveConfig();
}

bool NetworkService::clearWifiCredentials() {
    _wifiSSID = "";
    _wifiPassword = "";
    _configData.setWifiCredentials("", "");
    return _configData.saveConfig();
}

bool NetworkService::applyAndSaveConfig(const JsonVariantConst& flatValues) {
    _configData.applyParameterValues(flatValues);

    // Keep runtime WiFi fields aligned with values loaded into ConfigData.
    _wifiSSID = _configData.getWifiSsid();
    _wifiPassword = _configData.getWifiPassword();

    return _configData.saveConfig();
}

const std::vector<ConfigParameter>& NetworkService::getParameters() const {
    return _configData.getParameters();
}

static String encryptionTypeName(uint8_t type) {
    switch (type) {
        case ENC_TYPE_WEP:
            return "WEP";
        case ENC_TYPE_TKIP:
            return "WPA";
        case ENC_TYPE_CCMP:
            return "WPA2";
        case ENC_TYPE_NONE:
            return "OPEN";
        case ENC_TYPE_AUTO:
            return "AUTO";
        default:
            return "UNKNOWN";
    }
}

std::vector<WiFiScanResult> NetworkService::scanSsids() {
    Serial.println("[NetworkService] scanSsids() called");

    if (_scanInProgress) {
        int scanResult = WiFi.scanComplete();
        if (scanResult >= 0) {
            Serial.println("[NetworkService] async scan completed");
            _scanCache.clear();
            for (int i = 0; i < scanResult; ++i) {
                WiFiScanResult result;
                result.ssid = WiFi.SSID(i);
                result.rssi = WiFi.RSSI(i);
                result.encryption = encryptionTypeName(WiFi.encryptionType(i));
                result.bssid = WiFi.BSSIDstr(i);
                result.channel = WiFi.channel(i);
                result.hidden = WiFi.isHidden(i);
                _scanCache.push_back(result);
            }
            WiFi.scanDelete();
            _scanInProgress = false;
            _lastScanTimestamp = millis();
        } else {
            Serial.println("[NetworkService] async scan still running, returning cached results");
            return _scanCache;
        }
    }

    if (!_scanCache.empty() && (millis() - _lastScanTimestamp) < 60u * 1000u) {
        Serial.println("[NetworkService] returning cached scan results");
        return _scanCache;
    }

    Serial.println("[NetworkService] starting async scan");
    WiFi.mode(WIFI_AP_STA);
    int scanResult = WiFi.scanNetworks(true);

    if (scanResult >= 0) {
        Serial.println("[NetworkService] sync scan completed immediately");
        _scanCache.clear();
        for (int i = 0; i < scanResult; ++i) {
            WiFiScanResult result;
            result.ssid = WiFi.SSID(i);
            result.rssi = WiFi.RSSI(i);
            result.encryption = encryptionTypeName(WiFi.encryptionType(i));
            result.bssid = WiFi.BSSIDstr(i);
            result.channel = WiFi.channel(i);
            result.hidden = WiFi.isHidden(i);
            _scanCache.push_back(result);
        }
        WiFi.scanDelete();
        _scanInProgress = false;
        _lastScanTimestamp = millis();
    } else {
        _scanInProgress = true;
        _lastScanTimestamp = millis();
    }

    return _scanCache;
}

bool NetworkService::isScanActive() const {
    return _scanInProgress;
}
