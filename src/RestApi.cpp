#include "NetworkService.h"
#include "RestApi.h"
#include <ArduinoJson.h>

RestApi::RestApi(NetworkService& configService, AsyncWebServer& server)
    : _networkService(configService), _server(server) {
}

void RestApi::setup() {
    // Simple ping endpoint to verify API reachability from clients
    _server.on("/api/ping", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("[RestApi] /api/ping requested");
        request->send(200, "application/json", "{\"ok\":true}");
    });
    Serial.println("[RestApi] Endpoints registered");
    setupConfigEndpoint();
    setupStatusEndpoint();
    setupScanEndpoint();
    setupSaveEndpoint();

}

void RestApi::setupConfigEndpoint() {
    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument responseDoc;
        String apSsid = _networkService.getApSsid();
        responseDoc["page_title"] = _networkService.getPageTitle();
        responseDoc["mdns_hostname"] = _networkService.getMdnsHostname();
        responseDoc["wifi_mode"] = apSsid == "" ? "STA" : "AP";
        responseDoc["ap_ssid"] = apSsid;
        responseDoc["wifi_ssid"] = _networkService.getWifiSsid();

        JsonArray params = responseDoc["parameters"].to<JsonArray>();
        for (const auto& param : _networkService.getParameters()) {
            if (!param.valuePointer) {
                continue;
            }

            JsonObject paramObj = params.add<JsonObject>();
            paramObj["id"] = param.id;
            paramObj["label"] = param.label;
            switch (param.type) {
                case ParamType::INT: {
                    int* value = static_cast<int*>(param.valuePointer);
                    paramObj["type"] = "int";
                    paramObj["value"] = *value;
                    break;
                }
                case ParamType::FLOAT: {
                    float* value = static_cast<float*>(param.valuePointer);
                    paramObj["type"] = "float";
                    paramObj["value"] = *value;
                    break;
                }
                case ParamType::STRING: {
                    String* value = static_cast<String*>(param.valuePointer);
                    paramObj["type"] = "string";
                    paramObj["value"] = *value;
                    break;
                }
                case ParamType::BOOL: {
                    bool* value = static_cast<bool*>(param.valuePointer);
                    paramObj["type"] = "bool";
                    paramObj["value"] = *value;
                    break;
                }
            }
        }

        String responseBody;
        ArduinoJson::serializeJson(responseDoc, responseBody);
        request->send(200, "application/json", responseBody);
    });
}

void RestApi::setupStatusEndpoint() {
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
    });

}

void RestApi::setupSaveEndpoint() {
    _server.on("/api/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // Logic to get POST data, update _parameters and call saveConfig()
        request->send(200, "text/html", "<h1>Saved! Restarting...</h1>");
        delay(2000);
        ESP.restart();
    });
}

void RestApi::setupScanEndpoint() {
    _server.on("/api/ssids", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("[RestApi] /api/ssids requested");
        StaticJsonDocument<2048> responseDoc;
        JsonArray ssidArray = responseDoc["ssids"].to<JsonArray>();
        for (const auto& result : _networkService.scanSsids()) {
            JsonObject item = ssidArray.add<JsonObject>();
            item["ssid"] = result.ssid;
            item["rssi"] = result.rssi;
            item["channel"] = result.channel;
            item["encryption"] = result.encryption;
            item["bssid"] = result.bssid;
            item["hidden"] = result.hidden;
        }
        responseDoc["scanning"] = _networkService.isScanActive();

        String responseBody;
        serializeJson(responseDoc, responseBody);
        request->send(200, "application/json", responseBody);
    });
}