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
    // Do not start the server here; caller will start it after REST endpoints
    // are registered so handlers are active before the server begins.
}

void CaptivePortal::setupStationMode(const String& mdnsHostname) {
    _dnsActive = false;
    startMdns(mdnsHostname);
    setupRoutes();
}

void CaptivePortal::handle() {
    if (_dnsActive) {
        _dnsServer.processNextRequest();
    }

    MDNS.update();

    // Log when a client connects/disconnects to the soft-AP
    if (WiFi.getMode() & WIFI_AP) {
        int stationCount = WiFi.softAPgetStationNum();
        if (stationCount != _lastStationCount) {
            Serial.print("[CaptivePortal] soft-AP connected stations: ");
            Serial.println(stationCount);
            _lastStationCount = stationCount;
        }
    }
}

void CaptivePortal::setupRoutes() {
    if (_routesConfigured) {
        return;
    }

    // Allows mDNS requests
    MDNS.addService("http", "tcp", 80);

    auto& server = _server;

    // Explicitly serve the index page at root, and static assets only from /cm and /images.
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/cm/index.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    // Compatibility route: the portal is served from "/", so relative
    // references like "app.js" resolve to "/app.js" on clients.
    _server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/cm/app.js", "application/javascript");
    });

    _server.serveStatic("/cm", LittleFS, "/cm");
    _server.serveStatic("/images", LittleFS, "/images");

    // Catch-all route to force Captive Portal on mobile devices
    // Only redirect typical browser HTML requests to the portal root.
    // Allow API and asset requests to return their normal responses.
    server.onNotFound([&server](AsyncWebServerRequest *request) {
        // Log all unmatched requests so we can debug why API handlers may not be reached
        Serial.print("[CaptivePortal] onNotFound URL: ");
        Serial.println(request->url());

        // Don't force-redirect API or asset requests to the portal root.
        String url = request->url();
        if (url.startsWith("/api/") || url.startsWith("/images/") || url.startsWith("/cm/") || url.endsWith(".js") || url.endsWith(".css") || url.endsWith(".png") || url.endsWith(".jpg") || url.endsWith(".gif") || url.indexOf('.') >= 0) {
            request->send(404, "text/plain", "Not found");
            return;
        }

        // For regular browser navigations, redirect to the portal root so mobile devices show captive portal UI.
        request->redirect("/");
    });

    _routesConfigured = true;
}

AsyncWebServer& CaptivePortal::getServer() {
    return _server;
}

void CaptivePortal::startAP(const String& apSSID, const String& mdnsHostname) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str(), ""); // Open network for captive portal

    // Log AP details so testers know which SSID/IP to connect to
    Serial.print("[CaptivePortal] AP started: ");
    Serial.print(apSSID);
    Serial.print("  IP: ");
    Serial.println(WiFi.softAPIP());

    // Configures DNS to redirect ALL traffic to the NodeMCU IP (Captive Portal)
    _dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    _dnsActive = true;

    // Starts mDNS for access via friendly hostname (e.g., http://configmanager.local)
    startMdns(mdnsHostname);
}

void CaptivePortal::startMdns(const String& mdnsHostname) {
    if (!MDNS.begin(mdnsHostname.c_str())) {
        Serial.println("[CaptivePortal] Error starting mDNS");
    } else {
        Serial.print("[CaptivePortal] mDNS started: http://");
        Serial.print(mdnsHostname);
        Serial.println(".local");
    }
}