#ifndef CORE_REST_API_H
#define CORE_REST_API_H

#include <Arduino.h>
#include <ESP8266WebServer.h>

// Forward declaration
class NetworkService;

/**
 * @brief Dedicated class for handling REST API endpoints.
 * Provides JSON API access to ConfigManager state and functionality.
 */
class RestApi {
public:
    /**
     * @brief Constructs a RestApi instance.
     * @param configService Reference to the ConfigService instance
     * @param server Reference to the web server instance
     */
    RestApi(NetworkService& configService, ESP8266WebServer& server);

    /**
     * @brief Sets up all REST API endpoints.
     */
    void setup();

private:
    NetworkService& _configService;
    ESP8266WebServer& _server;

    void setupConfigEndpoint();
    void setupStatusEndpoint();
    void setupSaveEndpoint();
};

#endif // CORE_REST_API_H