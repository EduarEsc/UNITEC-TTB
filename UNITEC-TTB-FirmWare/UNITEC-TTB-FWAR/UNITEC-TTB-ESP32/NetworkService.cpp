#include "NetworkService.h"
#include <WiFi.h>
#include <ArduinoJson.h>
// Incluimos los servicios para que NetworkService pueda darles órdenes
#include "AudioService.h"
#include "LedService.h"
#include "MicService.h"

// --- CONFIGURACIÓN DE RED ---
const char* ssid = "INFINITUM09F7";
const char* password = "1q80IK50Qd";
// Esta IP debe ser la de tu laptop corriendo el backend
const char* server_url_base = "http://192.168.1.70:8000/audios/"; 

void setupNetwork() {
    Serial.println("{\"type\":\"SYSTEM\", \"status\":\"WIFI_CONNECTING\"}");
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    // Aumentamos un poco el tiempo de espera por si el router está lejos
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        attempts++;
        if(attempts % 5 == 0) Serial.print("."); 
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.printf("{\"type\":\"SYSTEM\", \"status\":\"WIFI_CONNECTED\", \"ip\":\"%s\"}\n", WiFi.localIP().toString().c_str());
        
        // Bienvenida inicial: Da tiempo a que el buffer del DAC se estabilice
        delay(1500); 
        audioService.play("1_bienvenida", false); 
        
        // Estado visual: Por ejemplo, el 1 (BIENVENIDA/REGISTRO)
        ledService.setEstado(1); 
    } else {
        Serial.println("\n{\"type\":\"ERROR\", \"msg\":\"WIFI_FAILED\"}");
    }
}

void updateNetwork() {
    // 1. Si el Mic está enviando audio binario por Serial, NO leemos JSON
    // para no corromper el flujo de bytes que va hacia la PC.
    if (micService.isStreaming()) return;

    // 2. Mantener la conexión WiFi viva
    if (WiFi.status() != WL_CONNECTED) {
        // Podrías intentar reconectar aquí si fuera necesario
    }

    // 3. Leer comandos del Backend (Python -> ESP32)
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        if (input.length() > 0 && input.startsWith("{")) {
            processLocalMessage(input);
        }
    }
}

void sendHardwareAction(String action, String value) {
    // Usamos StaticJsonDocument para enviar eventos al Backend (ESP32 -> Python)
    StaticJsonDocument<128> doc;
    doc["type"] = "HW_EVENT";
    doc["action"] = action;
    doc["value"] = value;
    
    serializeJson(doc, Serial);
    Serial.println(); // Importante para que el 'readline' de Python detecte el fin
}

void processLocalMessage(String jsonPayload) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonPayload);

    if (error) {
        Serial.print("{\"type\":\"ERROR\", \"msg\":\"JSON_PARSE_FAILED\"}\n");
        return;
    }

    String type = doc["type"] | "";

    // COMANDO: Reproducir Audio
    if (type == "CMD_AUDIO") {
        String file = doc["file"] | "";
        bool loop = doc["loop"] | false;
        if (file != "") audioService.play(file, loop);
    } 
    // COMANDO: Cambiar Luces
    else if (type == "CMD_LED") {
        int estado = doc["state"] | 0;
        ledService.setEstado(estado);
    }
    // COMANDO: Detener todo (Emergencia/Reset)
    else if (type == "CMD_STOP_ALL") {
        audioService.stop();
        micService.stopStreaming();
        // ledService.setEstado(0); // Opcional: volver a IDLE
    }
    // COMANDO: Secuencia de audios (Para frases compuestas)
    else if (type == "CMD_SEQUENCE") {
        JsonArray files = doc["files"];
        std::vector<String> sequence;
        for (JsonVariant f : files) {
            sequence.push_back(f.as<String>());
        }
        audioService.playSequence(sequence);
    }
}