#include "CaptivePortal.h"
#include "NetworkService.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

const byte DNS_PORT = 53;

CaptivePortal::CaptivePortal(NetworkService& configService)
    : _configService(configService), _server(80) {
}

void CaptivePortal::setup(const String& apSSID, const String& mdnsHostname) {
    startAP(apSSID, mdnsHostname);
    setupRoutes();
    _server.begin();
}

void CaptivePortal::handle() {
    _dnsServer.processNextRequest();
    _server.handleClient();
}

void CaptivePortal::setupRoutes() {
    // Allows mDNS requests
    MDNS.addService("http", "tcp", 80);

    auto& server = _server;

    // Root route (loads compressed HTML from file system)
    server.on("/", [&server]() {
        File file = LittleFS.open("/index.html.gz", "r");
        if (file) {
            server.sendHeader("Content-Encoding", "gzip");
            server.streamFile(file, "text/html");
            file.close();
            return;
        }

        file = LittleFS.open("/index.html", "r");
        if (file) {
            server.streamFile(file, "text/html");
            file.close();
            return;
        }

        server.send(200, "text/html", "<h1>Welcome to Configuration</h1><p>Access via: <b>http://configmanager.local</b></p>");
    });

    // Catch-all route to force Captive Portal on mobile devices
    server.onNotFound([&server]() {
        server.sendHeader("Location", "/", true); // Redirects to root
        server.send(302, "text/plain", "");
    });
}

ESP8266WebServer& CaptivePortal::getServer() {
    return _server;
}

void CaptivePortal::startAP(const String& apSSID, const String& mdnsHostname) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str(), ""); // Open network for captive portal

    // Configures DNS to redirect ALL traffic to the NodeMCU IP (Captive Portal)
    _dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // Starts mDNS for access via friendly hostname (e.g., http://configmanager.local)
    if (!MDNS.begin(mdnsHostname.c_str())) {
        Serial.println("[CaptivePortal] Error starting mDNS");
    } else {
        Serial.print("[CaptivePortal] mDNS started: http://");
        Serial.print(mdnsHostname);
        Serial.println(".local");
    }
}