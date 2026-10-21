#include "NetworkService.h"
#include "RestApi.h"
#include <ArduinoJson.h>

static String decodeUrlEncodedValue(AsyncWebServerRequest *request, const String& value) {
    return request->urlDecode(value);
}

static void parseConnectRequestBody(AsyncWebServerRequest *request, const String& body, String& ssid, String& password) {
    ssid = "";
    password = "";

    if (body.isEmpty()) {
        return;
    }

    if (request->contentType().indexOf("application/json") >= 0) {
        JsonDocument doc;
        if (deserializeJson(doc, body) == DeserializationError::Ok) {
            ssid = doc["ssid"].as<String>();
            password = doc["password"].as<String>();
        }
        return;
    }

    int start = 0;
    while (start < static_cast<int>(body.length())) {
        int separator = body.indexOf('&', start);
        if (separator < 0) {
            separator = body.length();
        }

        int equals = body.indexOf('=', start);
        if (equals >= 0 && equals < separator) {
            String key = decodeUrlEncodedValue(request, body.substring(start, equals));
            String value = decodeUrlEncodedValue(request, body.substring(equals + 1, separator));

            if (key == "ssid") {
                ssid = value;
            } else if (key == "password") {
                password = value;
            }
        }

        start = separator + 1;
    }
}

static void logConnectRequest(AsyncWebServerRequest *request, const String* rawBody) {
    Serial.println("[RestApi] /api/connect request received");
    Serial.print("[RestApi] method: ");
    Serial.println(request->methodToString());
    Serial.print("[RestApi] content-type: ");
    Serial.println(request->contentType());
    Serial.print("[RestApi] content-length: ");
    Serial.println(request->contentLength());
    Serial.print("[RestApi] params count: ");
    Serial.println(request->params());

    for (size_t index = 0; index < request->params(); ++index) {
        const AsyncWebParameter* param = request->getParam(index);
        if (param == nullptr) {
            continue;
        }

        Serial.print("[RestApi] param[");
        Serial.print(index);
        Serial.print("] name=");
        Serial.print(param->name());
        Serial.print(" value=");
        Serial.print(param->value());
        Serial.print(" isPost=");
        Serial.print(param->isPost() ? "true" : "false");
        Serial.print(" isFile=");
        Serial.println(param->isFile() ? "true" : "false");
    }

    Serial.print("[RestApi] hasParam(ssid,false): ");
    Serial.println(request->hasParam("ssid", false) ? "true" : "false");
    Serial.print("[RestApi] hasParam(ssid,true): ");
    Serial.println(request->hasParam("ssid", true) ? "true" : "false");
    Serial.print("[RestApi] hasArg(ssid): ");
    Serial.println(request->hasArg("ssid") ? "true" : "false");

    if (rawBody != nullptr) {
        Serial.print("[RestApi] raw body: ");
        Serial.println(*rawBody);
    } else {
        Serial.println("[RestApi] raw body: <null>");
    }
}

static void extractConnectRequestFields(AsyncWebServerRequest *request, const String* rawBody, String& ssid, String& password) {
    ssid = "";
    password = "";

    if (rawBody != nullptr) {
        parseConnectRequestBody(request, *rawBody, ssid, password);
    }

    if (ssid.isEmpty() && request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }
    if (password.isEmpty() && request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    }

    if (ssid.isEmpty() && request->hasParam("ssid", false)) {
        ssid = request->getParam("ssid", false)->value();
    }
    if (password.isEmpty() && request->hasParam("password", false)) {
        password = request->getParam("password", false)->value();
    }

    if (ssid.isEmpty() && request->hasArg("ssid")) {
        ssid = request->arg("ssid");
    }
    if (password.isEmpty() && request->hasArg("password")) {
        password = request->arg("password");
    }
}

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
    setupConnectEndpoint();
    setupStatusEndpoint();
    setupScanEndpoint();
    setupSaveEndpoint();

}

void RestApi::setupConfigEndpoint() {
    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument responseDoc;
        responseDoc["page_title"] = _networkService.getPageTitle();
        responseDoc["mdns_hostname"] = _networkService.getMdnsHostname();
        responseDoc["wifi_mode"] = _networkService.getWifiMode();
        responseDoc["ap_ssid"] = _networkService.getApSsid();
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
                case ParamType::UINT32: {
                    uint32_t* value = static_cast<uint32_t*>(param.valuePointer);
                    paramObj["type"] = "uint32";
                    paramObj["value"] = *value;
                    break;
                }
                case ParamType::ULONG: {
                    unsigned long* value = static_cast<unsigned long*>(param.valuePointer);
                    paramObj["type"] = "ulong";
                    paramObj["value"] = *value;
                    break;
                }
                case ParamType::FLOAT: {
                    float* value = static_cast<float*>(param.valuePointer);
                    paramObj["type"] = "float";
                    paramObj["value"] = *value;
                    break;
                }
                case ParamType::DOUBLE: {
                    double* value = static_cast<double*>(param.valuePointer);
                    paramObj["type"] = "double";
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

void RestApi::setupConnectEndpoint() {
    _server.on(
        "/api/connect",
        HTTP_POST,
        [this](AsyncWebServerRequest *request) {
            String* rawBody = static_cast<String*>(request->_tempObject);
            String ssid;
            String password;

            logConnectRequest(request, rawBody);
            extractConnectRequestFields(request, rawBody, ssid, password);

            Serial.print("[RestApi] parsed ssid: ");
            Serial.println(ssid.isEmpty() ? "<empty>" : ssid);
            Serial.print("[RestApi] parsed password length: ");
            Serial.println(password.length());

            if (rawBody != nullptr) {
                delete rawBody;
                request->_tempObject = nullptr;
            }

            if (ssid.isEmpty()) {
                Serial.println("[RestApi] rejecting connect request because SSID is empty");
                request->send(400, "application/json", "{\"success\":false,\"error\":\"SSID is required.\"}");
                return;
            }

            Serial.print("[RestApi] /api/connect requested for SSID: ");
            Serial.println(ssid);

            if (!_networkService.saveWifiCredentials(ssid, password)) {
                request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save WiFi credentials.\"}");
                return;
            }

            Serial.println("[RestApi] WiFi credentials saved successfully, restarting for STA reconnect");

            String responseBody = "{\"success\":true,\"message\":\"Credentials saved for " + ssid + ". Restarting...\"}";
            request->send(200, "application/json", responseBody);
            delay(1000);
            ESP.restart();
        },
        nullptr,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String* rawBody = static_cast<String*>(request->_tempObject);
            if (rawBody == nullptr) {
                rawBody = new String();
                request->_tempObject = rawBody;
                Serial.println("[RestApi] /api/connect body collection started");
            }

            if (index == 0) {
                rawBody->reserve(total);
                Serial.print("[RestApi] /api/connect expected body bytes: ");
                Serial.println(total);
            }

            for (size_t i = 0; i < len; ++i) {
                rawBody->concat(static_cast<char>(data[i]));
            }

            Serial.print("[RestApi] /api/connect body chunk index=");
            Serial.print(index);
            Serial.print(" len=");
            Serial.print(len);
            Serial.print(" accumulated=");
            Serial.println(rawBody->length());
        }
    );
}

void RestApi::setupStatusEndpoint() {
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument responseDoc;
        JsonArray items = responseDoc.to<JsonArray>();

        JsonArray wifiSignal = items.add<JsonArray>();
        wifiSignal.add("WiFi Signal");
        wifiSignal.add(String(WiFi.RSSI()) + " dBm");

        JsonArray uptime = items.add<JsonArray>();
        uptime.add("Uptime");
        uptime.add(String(millis() / 1000) + " seconds");

        JsonArray hostname = items.add<JsonArray>();
        hostname.add("Hostname");
        hostname.add(_networkService.getMdnsHostname() + ".local");

        JsonArray ipAddress = items.add<JsonArray>();
        ipAddress.add("IP Address");
        ipAddress.add(WiFi.localIP().toString());

        String responseBody;
        serializeJson(responseDoc, responseBody);
        request->send(200, "application/json", responseBody);
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