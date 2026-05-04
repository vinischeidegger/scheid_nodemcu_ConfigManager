#include "NetworkService.h"
#include "RestApi.h"
#include <ArduinoJson.h>

RestApi::RestApi(NetworkService& configService, ESP8266WebServer& server)
    : _configService(configService), _server(server) {
}

void RestApi::setup() {
    setupConfigEndpoint();
    setupStatusEndpoint();
    setupSaveEndpoint();
}

void RestApi::setupConfigEndpoint() {
    _server.on("/api/config", HTTP_GET, [this]() {
        JsonDocument responseDoc;
        responseDoc["page_title"] = _configService.getPageTitle();
        responseDoc["mdns_hostname"] = _configService.getMdnsHostname();
        responseDoc["ap_ssid"] = _configService.getApSsid();
        responseDoc["wifi_ssid"] = _configService.getWifiSsid();

        JsonArray params = responseDoc["parameters"].to<JsonArray>();
        for (const auto& param : _configService.getParameters()) {
            JsonObject paramObj = params.add<JsonObject>();
            paramObj["id"] = param.id;
            paramObj["label"] = param.label;
            switch (param.type) {
                case ParamType::INT:
                    paramObj["type"] = "int";
                    paramObj["value"] = *(int*)param.valuePointer;
                    break;
                case ParamType::FLOAT:
                    paramObj["type"] = "float";
                    paramObj["value"] = *(float*)param.valuePointer;
                    break;
                case ParamType::STRING:
                    paramObj["type"] = "string";
                    paramObj["value"] = *(String*)param.valuePointer;
                    break;
                case ParamType::BOOL:
                    paramObj["type"] = "bool";
                    paramObj["value"] = *(bool*)param.valuePointer;
                    break;
            }
        }

        String responseBody;
        serializeJson(responseDoc, responseBody);
        _server.send(200, "application/json", responseBody);
    });
}

void RestApi::setupStatusEndpoint() {
    auto& server = _server;

    server.on("/api/status", HTTP_GET, [&server]() {
        server.send(200, "text/plain", "OK");
    });

    server.on("/status", HTTP_GET, [&server]() {
        server.send(200, "text/plain", "OK");
    });
}

void RestApi::setupSaveEndpoint() {
    _server.on("/salvar", HTTP_POST, [this]() {
        // Logic to get POST data, update _parameters and call saveConfig()
        _server.send(200, "text/html", "<h1>Saved! Restarting...</h1>");
        delay(2000);
        ESP.restart();
    });
}