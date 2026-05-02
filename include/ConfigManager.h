#ifndef CORE_CONFIG_MANAGER_H
#define CORE_CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>

/**
 * @brief Tipos de dados suportados para os parâmetros de configuração.
 */
enum class ParamType {
    INT,
    FLOAT,
    STRING,
    BOOL
};

/**
 * @brief Estrutura interna para mapear variáveis do produto para o ConfigManager.
 */
struct ConfigParameter {
    String id;
    void* valuePointer;
    ParamType type;
    String label;
};

class ConfigManager {
public:
    ConfigManager();
    
    /**
     * @brief Registra um parâmetro que será gerenciado pelo portal e salvo na memória.
     * @param id Identificador único no JSON (ex: "setpoint_pressao")
     * @param valuePointer Ponteiro para a variável no seu código principal
     * @param type Tipo do dado (ParamType)
     * @param label Nome amigável que aparecerá no formulário HTML do celular
     */
    void registerParameter(const String& id, void* valuePointer, ParamType type, const String& label);

    /**
     * @brief Inicializa o sistema, monta o sistema de arquivos e tenta carregar configs.
     * Se não conseguir conectar ao WiFi salvo, entra em modo Access Point automaticamente.
     */
    bool begin();

    /**
     * @brief Deve ser chamado dentro do loop() principal para manter o WebServer e DNS ativos.
     */
    void handle();

    /**
     * @brief Salva os valores atuais das variáveis registradas no sistema de arquivos (LittleFS).
     */
    bool saveConfig();

    /**
     * @brief Carrega os valores do arquivo JSON para as variáveis registradas.
     */
    bool loadConfig();

    /**
     * @brief Reinicia as configurações de rede (Limpa WiFi e entra em modo AP).
     */
    void resetNetwork();

private:
    std::vector<ConfigParameter> _parameters;
    String _apSSID;
    String _apPassword;
    
    // Configurações de Rede Internas
    String _wifiSSID;
    String _wifiPassword;

    // Métodos Privados
    void setupCaptivePortal();
    void startAP();
    bool connectWiFi();
    
    // Constantes de Caminho
    const char* _configPath = "/config.json";
};

#endif // CORE_CONFIG_MANAGER_H