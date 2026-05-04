#ifndef CORE_NETWORK_SERVICE_H
#define CORE_NETWORK_SERVICE_H

#include <Arduino.h>
#include "ConfigData.h"

#ifndef ARDUINO
#include "../test/mocks/CaptivePortal.h"
#include "../test/mocks/RestApi.h"
#else
#include "CaptivePortal.h"
#include "RestApi.h"
#endif

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

private:
    ConfigData& _configData;
    String _pageTitle = "Scheid Product Configuration";
    String _apSSID;
    String _wifiSSID;
    String _wifiPassword;
    String _mdnsHostname;

    CaptivePortal* _captivePortal = nullptr;
    RestApi* _restApi = nullptr;

    // Private Methods
    bool connectWiFi();
};

#endif // CORE_NETWORK_SERVICE_H