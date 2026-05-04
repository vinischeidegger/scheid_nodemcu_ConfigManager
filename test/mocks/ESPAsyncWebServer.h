#ifndef MOCK_ESP_ASYNC_WEB_SERVER_H
#define MOCK_ESP_ASYNC_WEB_SERVER_H

class AsyncWebServer {
public:
    AsyncWebServer(int port = 80) {}
    void on(const char*, int, void(*)()) {}
    void begin() {}
};

#endif // MOCK_ESP_ASYNC_WEB_SERVER_H
