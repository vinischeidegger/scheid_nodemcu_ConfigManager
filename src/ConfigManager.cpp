#include "ConfigManager.h"
#include <LittleFS.h>

// Bibliotecas de Rede (Focadas no ESP8266/NodeMCU para este exemplo)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

// Instâncias globais para o Servidor Web e DNS (mantidas fora do .h para não poluir os produtos finais)
ESP8266WebServer server(80);
DNSServer dnsServer;

const byte DNS_PORT = 53;

ConfigManager::ConfigManager() {
    // Configurações padrão de fábrica
    _apSSID = "Produto_Config"; 
    _apPassword = ""; // Rede Aberta para o Captive Portal
    _wifiSSID = "";
    _wifiPassword = "";
}

void ConfigManager::registerParameter(const String& id, void* valuePointer, ParamType type, const String& label) {
    // Adiciona o parâmetro do produto à nossa lista interna
    _parameters.push_back({id, valuePointer, type, label});
}

bool ConfigManager::begin() {
    Serial.println("[ConfigManager] Inicializando...");

    // 1. Inicia o sistema de arquivos
    if (!LittleFS.begin()) {
        Serial.println("[ConfigManager] Falha ao montar LittleFS. Formatando...");
        LittleFS.format(); // Tenta formatar na primeira vez que a placa for usada
        LittleFS.begin();
    }

    // 2. Carrega as configurações (se existirem) para as variáveis registradas
    loadConfig();

    // 3. Tenta conectar ao WiFi
    if (_wifiSSID != "" && connectWiFi()) {
        Serial.println("[ConfigManager] Conectado à rede WiFi com sucesso!");
        return true;
    }

    // 4. Se falhou ou não tem WiFi configurado, levanta o Captive Portal
    Serial.println("[ConfigManager] Iniciando modo AP e Captive Portal...");
    startAP();
    setupCaptivePortal();
    return false;
}

void ConfigManager::handle() {
    // Mantém o servidor Web e o DNS rodando em background
    dnsServer.processNextRequest();
    server.handleClient();
}

bool ConfigManager::saveConfig() {
    Serial.println("[ConfigManager] Salvando configurações...");

    // Usando ArduinoJson v7 (JsonDocument dinâmico e otimizado)
    JsonDocument doc; 

    // Salva as credenciais de rede
    doc["wifi_ssid"] = _wifiSSID;
    doc["wifi_pass"] = _wifiPassword;

    // Salva dinamicamente TODOS os parâmetros registrados pelos produtos
    for (const auto& param : _parameters) {
        switch (param.type) {
            // Fazemos o "cast" do ponteiro void* de volta para o tipo original
            case ParamType::INT:    doc[param.id] = *(int*)param.valuePointer; break;
            case ParamType::FLOAT:  doc[param.id] = *(float*)param.valuePointer; break;
            case ParamType::STRING: doc[param.id] = *(String*)param.valuePointer; break;
            case ParamType::BOOL:   doc[param.id] = *(bool*)param.valuePointer; break;
        }
    }

    // Abre o arquivo e grava
    File file = LittleFS.open(_configPath, "w");
    if (!file) return false;

    serializeJson(doc, file);
    file.close();
    return true;
}

bool ConfigManager::loadConfig() {
    Serial.println("[ConfigManager] Carregando configurações...");

    File file = LittleFS.open(_configPath, "r");
    if (!file) return false; // Arquivo não existe ainda

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("[ConfigManager] Falha ao ler JSON.");
        return false;
    }

    // Carrega credenciais de rede
    if (doc.containsKey("wifi_ssid")) _wifiSSID = doc["wifi_ssid"].as<String>();
    if (doc.containsKey("wifi_pass")) _wifiPassword = doc["wifi_pass"].as<String>();

    // Atualiza dinamicamente as variáveis lá no main.cpp do produto
    for (const auto& param : _parameters) {
        if (doc.containsKey(param.id)) {
            switch (param.type) {
                case ParamType::INT:    *(int*)param.valuePointer = doc[param.id].as<int>(); break;
                case ParamType::FLOAT:  *(float*)param.valuePointer = doc[param.id].as<float>(); break;
                case ParamType::STRING: *(String*)param.valuePointer = doc[param.id].as<String>(); break;
                case ParamType::BOOL:   *(bool*)param.valuePointer = doc[param.id].as<bool>(); break;
            }
        }
    }
    return true;
}

// ==========================================
// MÉTODOS DE REDE (Implementação Básica)
// ==========================================

bool ConfigManager::connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifiSSID.c_str(), _wifiPassword.c_str());
    
    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) { // Timeout de ~10 segundos
        delay(500);
        Serial.print(".");
        tentativas++;
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

void ConfigManager::startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_apSSID.c_str(), _apPassword.c_str());
    
    // Configura o DNS para redirecionar TODO o tráfego para o IP do NodeMCU (Captive Portal)
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

void ConfigManager::setupCaptivePortal() {
    // Rota raiz (onde construiremos o HTML)
    server.on("/", []() {
        server.send(200, "text/html", "<h1>Bem vindo as Configuracoes</h1><p>Em breve, geraremos o form aqui!</p>");
    });

    // Rota para pegar os dados do formulário quando o usuário apertar "Salvar"
    server.on("/salvar", HTTP_POST, [this]() {
        // Lógica para pegar os dados do POST, atualizar _parameters e chamar saveConfig()
        server.send(200, "text/html", "<h1>Salvo! Reiniciando...</h1>");
        delay(2000);
        ESP.restart();
    });

    // Rota coringa para forçar o Captive Portal nos celulares
    server.onNotFound([]() {
        server.sendHeader("Location", "/", true); // Redireciona para a raiz
        server.send(302, "text/plain", "");
    });

    server.begin();
}