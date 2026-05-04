#ifndef MOCK_CAPTIVE_PORTAL_H
#define MOCK_CAPTIVE_PORTAL_H

#include "mock_arduino.h"

class WiFiServer {
public:
    void begin() {}
};

class NetworkService;

class CaptivePortal {
public:
    explicit CaptivePortal(NetworkService& service) : _service(service) {}
    ~CaptivePortal() = default;

    void setup(const String& apSSID, const String& mdnsHostname) {
        (void)apSSID;
        (void)mdnsHostname;
    }

    void handle() {}
    WiFiServer& getServer() { return _server; }

private:
    NetworkService& _service;
    WiFiServer _server;
};

#endif // MOCK_CAPTIVE_PORTAL_H
