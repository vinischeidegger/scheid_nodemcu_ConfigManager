#ifndef CORE_CAPTIVE_PORTAL_H
#define CORE_CAPTIVE_PORTAL_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

// Forward declaration
class NetworkService;

/**
 * @brief Dedicated class for handling captive portal functionality.
 * Manages web server setup, HTML serving, and DNS redirection.
 */
class CaptivePortal {
public:
    /**
     * @brief Constructs a CaptivePortal instance.
     * @param configService Reference to the ConfigService instance
     */
    CaptivePortal(NetworkService& configService);

    /**
     * @brief Sets up the captive portal with web server and DNS.
     * @param apSSID Access Point SSID to use
     * @param mdnsHostname mDNS hostname for friendly access
     */
    void setup(const String& apSSID, const String& mdnsHostname);

    /**
     * @brief Must be called in the main loop to handle requests.
     */
    void handle();

    /**
     * @brief Provides access to the internal web server used by the portal.
     */
    AsyncWebServer& getServer();

private:
    NetworkService& _configService;
    AsyncWebServer _server;
    DNSServer _dnsServer;

    void setupRoutes();
    void startAP(const String& apSSID, const String& mdnsHostname);
};

#endif // CORE_CAPTIVE_PORTAL_H