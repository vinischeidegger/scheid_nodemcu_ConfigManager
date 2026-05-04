#ifndef MOCK_REST_API_H
#define MOCK_REST_API_H

#include "mock_arduino.h"

class WiFiServer;
class NetworkService;

class RestApi {
public:
    RestApi(NetworkService& service, WiFiServer& server) : _service(service), _server(server) {}
    ~RestApi() = default;

    void setup() {
        (void)_service;
        (void)_server;
    }

private:
    NetworkService& _service;
    WiFiServer& _server;
};

#endif // MOCK_REST_API_H
