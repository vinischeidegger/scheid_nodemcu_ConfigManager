#ifndef CORE_NETWORK_SERVICE_H
#define CORE_NETWORK_SERVICE_H

#include <Arduino.h>
#include "ConfigData.h"

#include "CaptivePortal.h"
#include "RestApi.h"

struct WiFiScanResult {
    String ssid;
    int rssi;
    String encryption;
    String bssid;
    int channel;
    bool hidden;
};

class NetworkService {
public:
    /**
     * @brief Constructs a ConfigService instance.
     * @param configData Reference to the ConfigData instance
     */
    NetworkService(ConfigData& configData);

    /**
     * @brief Destroys the ConfigService instance and releases internal resources.
     */
    ~NetworkService();

    /**
     * @brief Initializes the service, mounts the file system, and attempts to load configuration.
     * If unable to connect to saved WiFi, automatically enters Access Point mode.
     * @param apSSID Access Point SSID
     * @param wifiSSID WiFi SSID to connect to
     * @param wifiPassword WiFi password
     * @param mdnsHostname mDNS hostname
     */
    bool begin(const String& apSSID, const String& wifiSSID, const String& wifiPassword, const String& mdnsHostname);

    /**
     * @brief Must be called within the main loop() to keep the WebServer and DNS active.
     */
    void handle();

    /**
     * @brief Sets the mDNS hostname for friendly access (e.g., "configmanager.local").
     * @param hostname Name without ".local" (e.g., "configure" will result in "configure.local")
     */
    void setMdnsHostname(const String& hostname);

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
     * @brief Returns the current runtime WiFi mode label.
     */
    String getWifiMode() const;

    /**
     * @brief Returns the web portal page title.
     */
    String getPageTitle() const;

    /**
     * @brief Sets the web portal page title.
     * @param title Title text served by the REST API
     */
    void setPageTitle(const String& title);

    /**
     * @brief Returns the registered configuration parameters.
     */
    const std::vector<ConfigParameter>& getParameters() const;

    /**
     * @brief Tries to connect to the supplied WiFi credentials.
     * @param wifiSSID WiFi network name
     * @param wifiPassword WiFi password
     * @param keepAccessPoint True to keep AP mode active during the attempt
     */
    bool connectToWiFi(const String& wifiSSID, const String& wifiPassword, bool keepAccessPoint = false);

    /**
     * @brief Persists the supplied WiFi credentials to LittleFS.
     */
    bool saveWifiCredentials(const String& wifiSSID, const String& wifiPassword);

    /**
     * @brief Clears stored WiFi credentials so next boot falls back to AP mode.
     */
    bool clearWifiCredentials();

    /**
     * @brief Performs a WiFi network scan and returns discovered networks.
     * @note This function uses cached scan results and async scan to avoid
     *       blocking the WiFi server loop during long scans.
     */
    std::vector<WiFiScanResult> scanSsids();

    /**
     * @brief Returns true when a WiFi scan is currently in progress.
     */
    bool isScanActive() const;

private:
    ConfigData& _configData;
    String _pageTitle = "Scheid Product Configuration";
    String _apSSID;
    String _wifiSSID;
    String _wifiPassword;
    String _mdnsHostname;

    CaptivePortal* _captivePortal = nullptr;
    RestApi* _restApi = nullptr;

    // Cached scan state for /api/ssids.
    std::vector<WiFiScanResult> _scanCache;
    unsigned long _lastScanTimestamp = 0;
    bool _scanInProgress = false;

    // Private Methods
};

#endif // CORE_NETWORK_SERVICE_H