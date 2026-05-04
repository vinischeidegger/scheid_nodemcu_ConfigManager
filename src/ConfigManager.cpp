#include "ConfigManager.h"
#include "ConfigData.h"
#include "NetworkService.h"

ConfigManager::ConfigManager(const ConfigManagerOptions& options)
    : _configData(),
      _configService(_configData),
      _options(options) {
    // Set initial options
    _configService.setPageTitle(_options.pageTitle);
    // Note: WiFi credentials are not set here; they come from saved config or user input
    // But since NetworkService handles loading, we need to pass them
    // This is a simplification; ideally, NetworkService should load its own WiFi config
}

ConfigManager::~ConfigManager() {
    // NetworkService and ConfigData handle their own cleanup
}

void ConfigManager::registerParameter(const String& id, void* valuePointer, ParamType type, const String& label) {
    _configData.registerParameter(id, valuePointer, type, label);
}

bool ConfigManager::begin() {
    // Load saved configuration first
    _configData.loadConfig();

    // Use configured portal settings from options.
    // WiFi credentials still come from saved config or user input.
    return _configService.begin(_options.apSSID, "", "", _options.mdnsHostname);
}

void ConfigManager::handle() {
    _configService.handle();
}

bool ConfigManager::saveConfig() {
    return _configData.saveConfig();
}

bool ConfigManager::loadConfig() {
    return _configData.loadConfig();
}

void ConfigManager::resetNetwork() {
    // Implementation needed
}

void ConfigManager::setMdnsHostname(const String& hostname) {
    _configService.setMdnsHostname(hostname);
}

void ConfigManager::setPageTitle(const String& title) {
    _configService.setPageTitle(title);
}

String ConfigManager::getPageTitle() const {
    return _configService.getPageTitle();
}

String ConfigManager::getMdnsHostname() const {
    return _configService.getMdnsHostname();
}

String ConfigManager::getApSsid() const {
    return _configService.getApSsid();
}

String ConfigManager::getWifiSsid() const {
    return _configService.getWifiSsid();
}

const std::vector<ConfigParameter>& ConfigManager::getParameters() const {
    return _configData.getParameters();
}