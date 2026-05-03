#include <Arduino.h>
#include "ConfigManager.h"

// ==========================================
// 1. CONFIGURATION MODULE INSTANCE
// ==========================================
ConfigManager configManager;

// ==========================================
// 2. BUSINESS VARIABLES FOR THIS PRODUCT
// ==========================================
// These are the "factory" values. If there is anything already saved on the NodeMCU,
// ConfigManager will automatically overwrite these values in setup.
float setpointPressao = 3.5;       // Desired pressure in Bar
unsigned long tempoLeituraMs = 2000;         // Time between sensor readings
bool alarmeAtivo = true;           // Enable/Disable alarm
String nomeEquipamento = "Bomba 1"; 

// Variables for time control without blocking the processor
unsigned long ultimoTempoLeitura = 0;

void setup() {
    // Starts the serial port for debugging
    Serial.begin(115200);
    Serial.println("\n\n--- Starting Pressure Controller ---");

    // ==========================================
    // 3. PARAMETER REGISTRATION
    // ==========================================
    // Tell ConfigManager which variables it should manage and expose on the Captive Portal
    configManager.registerParameter("sp_pressao", &setpointPressao, ParamType::FLOAT, "Setpoint (Bar)");
    configManager.registerParameter("tempo_leitura", &tempoLeituraMs, ParamType::INT, "Tempo de Leitura (ms)");
    configManager.registerParameter("alarme", &alarmeAtivo, ParamType::BOOL, "Ativar Alarme");
    configManager.registerParameter("nome", &nomeEquipamento, ParamType::STRING, "Nome do Equipamento");

    // ==========================================
    // CONFIGURE mDNS HOSTNAME (OPTIONAL)
    // ==========================================
    // Defines a friendly hostname to access the portal. Default is "configmanager"
    // You can change to "configure", "pump", etc.
    configManager.setMdnsHostname("configure");

    // ==========================================
    // 4. BASE SYSTEM INITIALIZATION
    // ==========================================
    // This will mount LittleFS, load the JSON (if it exists), and attempt WiFi.
    // If WiFi is not available, it will silently raise the Access Point.
    configManager.begin();

    // ==========================================
    // 5. SPECIFIC HARDWARE SETUP
    // ==========================================
    // pinMode(PINO_SENSOR_PRESSAO, INPUT);
    // pinMode(PINO_RELE_BOMBA, OUTPUT);
    Serial.println("Hardware initialized. Entering main loop.");
}

void loop() {
    // ==========================================
    // THE GOLDEN RULE OF IOT ESP8266/ESP32
    // ==========================================
    // This function MUST be called in every cycle so the WebServer and DNS respond.
    configManager.handle();

    // ==========================================
    // PRODUCT LOGIC (NEVER USE delay() HERE)
    // ==========================================
    unsigned long tempoAtual = millis();

    // Executes the sensor reading based on the time configured by the user
    if (tempoAtual - ultimoTempoLeitura >= tempoLeituraMs) {
        ultimoTempoLeitura = tempoAtual;

        // Simulating the practical use of variables that Captive Portal manages:
        Serial.print("[");
        Serial.print(nomeEquipamento);
        Serial.print("] Current Setpoint: ");
        Serial.print(setpointPressao);
        Serial.print(" Bar | Alarm: ");
        Serial.println(alarmeAtivo ? "ON" : "OFF");

        /* Real logic would go here:
        float pressaoAtual = analogRead(PINO_SENSOR_PRESSAO) * FATOR_CONVERSAO;
        if (pressaoAtual < setpointPressao) {
            digitalWrite(PINO_RELE_BOMBA, HIGH);
        } else {
            digitalWrite(PINO_RELE_BOMBA, LOW);
        }
        */
    }
}