#include <Arduino.h>
#include "ConfigManager.h"

// ==========================================
// 1. INSTÂNCIA DO MÓDULO DE CONFIGURAÇÃO
// ==========================================
ConfigManager configManager;

// ==========================================
// 2. VARIÁVEIS DE NEGÓCIO DESTE PRODUTO
// ==========================================
// Estes são os valores "de fábrica". Se já houver algo salvo no NodeMCU, 
// o ConfigManager vai sobrescrever esses valores automaticamente no setup.
float setpointPressao = 3.5;       // Pressão desejada em Bar
unsigned long tempoLeituraMs = 2000;         // Tempo entre leituras do sensor
bool alarmeAtivo = true;           // Habilita/Desabilita sirene
String nomeEquipamento = "Bomba 1"; 

// Variáveis para controle de tempo sem travar o processador
unsigned long ultimoTempoLeitura = 0;

void setup() {
    // Inicia a porta serial para debug
    Serial.begin(115200);
    Serial.println("\n\n--- Iniciando Controlador de Pressao ---");

    // ==========================================
    // 3. REGISTRO DE PARÂMETROS
    // ==========================================
    // Avisamos ao ConfigManager quais variáveis ele deve gerenciar e expor no Captive Portal
    configManager.registerParameter("sp_pressao", &setpointPressao, ParamType::FLOAT, "Setpoint (Bar)");
    configManager.registerParameter("tempo_leitura", &tempoLeituraMs, ParamType::INT, "Tempo de Leitura (ms)");
    configManager.registerParameter("alarme", &alarmeAtivo, ParamType::BOOL, "Ativar Alarme");
    configManager.registerParameter("nome", &nomeEquipamento, ParamType::STRING, "Nome do Equipamento");

    // ==========================================
    // CONFIGURAR HOSTNAME mDNS (OPCIONAL)
    // ==========================================
    // Define um hostname amigável para acessar o portal. Padrão é "configmanager"
    // Você pode mudar para "configure", "bomba", etc.
    configManager.setMdnsHostname("configure");

    // ==========================================
    // 4. INICIALIZAÇÃO DO SISTEMA BASE
    // ==========================================
    // Isso vai montar o LittleFS, carregar o JSON (se existir) e tentar o WiFi.
    // Se não tiver WiFi, ele já levanta o Access Point silenciosamente.
    configManager.begin();

    // ==========================================
    // 5. SETUP ESPECÍFICO DO HARDWARE
    // ==========================================
    // pinMode(PINO_SENSOR_PRESSAO, INPUT);
    // pinMode(PINO_RELE_BOMBA, OUTPUT);
    Serial.println("Hardware inicializado. Entrando no loop principal.");
}

void loop() {
    // ==========================================
    // A REGRA DE OURO DO IOT ESP8266/ESP32
    // ==========================================
    // Esta função DEVE ser chamada em todo ciclo para o WebServer e o DNS responderem.
    configManager.handle();

    // ==========================================
    // LÓGICA DO PRODUTO (NUNCA USE delay() AQUI)
    // ==========================================
    unsigned long tempoAtual = millis();

    // Executa a leitura do sensor baseado no tempo configurado pelo usuário
    if (tempoAtual - ultimoTempoLeitura >= tempoLeituraMs) {
        ultimoTempoLeitura = tempoAtual;

        // Simulando o uso prático das variáveis que o Captive Portal gerencia:
        Serial.print("[");
        Serial.print(nomeEquipamento);
        Serial.print("] Setpoint atual: ");
        Serial.print(setpointPressao);
        Serial.print(" Bar | Alarme: ");
        Serial.println(alarmeAtivo ? "LIGADO" : "DESLIGADO");

        /* Lógica real viria aqui:
        float pressaoAtual = analogRead(PINO_SENSOR_PRESSAO) * FATOR_CONVERSAO;
        if (pressaoAtual < setpointPressao) {
            digitalWrite(PINO_RELE_BOMBA, HIGH);
        } else {
            digitalWrite(PINO_RELE_BOMBA, LOW);
        }
        */
    }
}