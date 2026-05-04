#ifndef MOCK_ESP8266_WIFI_H
#define MOCK_ESP8266_WIFI_H

#include "mock_arduino.h"

const int WIFI_STA = 1;
const int WL_CONNECTED = 3;

class WiFiClass {
public:
    void mode(int mode) {}
    void begin(const char* ssid, const char* password) {
        _status = WL_CONNECTED;
    }
    int status() const { return _status; }

private:
    int _status = WL_CONNECTED;
};

inline WiFiClass WiFi;

#endif // MOCK_ESP8266_WIFI_H
