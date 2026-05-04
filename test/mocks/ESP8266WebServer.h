#ifndef MOCK_ESP8266_WEB_SERVER_H
#define MOCK_ESP8266_WEB_SERVER_H

class ESP8266WebServer {
public:
    ESP8266WebServer() = default;
    void on(const char*, void(*)()) {}
    void begin() {}
    void send(int, const char*, const char*) {}
};

#endif // MOCK_ESP8266_WEB_SERVER_H
