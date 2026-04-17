#include "NetworkService.h"
#include <WiFi.h>
#include <ArduinoJson.h>

// =============================
// CONFIGURACIÓN DE RED
// =============================
//static const char* ssid = "More";
//static const char* password = "uden5775";
//static const char* server_url_base = "http://10.252.207.203:8000/audios/";
static const char* ssid = "INFINITUM09F7";
static const char* password = "1q80IK50Qd";
static const char* server_url_base = "http://192.168.1.70:8000/audios/";

static bool wifiConnected = false;
static String serialInputBuffer;

// =============================
// SETUP DE RED
// =============================
void setupNetwork() {
    Serial.println("{\"type\":\"SYSTEM\",\"event\":\"WIFI_CONNECTING\"}");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int attempts = 0;
    const int maxAttempts = 30;

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        attempts++;

        if (attempts % 5 == 0) {
            Serial.print(".");
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println();

        StaticJsonDocument<192> doc;
        doc["type"] = "SYSTEM";
        doc["event"] = "WIFI_CONNECTED";
        doc["ip"] = WiFi.localIP().toString();

        serializeJson(doc, Serial);
        Serial.println();
    } else {
        wifiConnected = false;
        Serial.println();
        Serial.println("{\"type\":\"ERROR\",\"event\":\"WIFI_FAILED\"}");
    }

    audioService.setServerBaseUrl(server_url_base);

    ledService.setEstado(VISTA_REGLAS);

    delay(300);
    audioService.play("1_bienvenida", false);
}

// =============================
// LOOP DE RED / SERIAL
// =============================
void updateNetwork() {
    // Durante streaming del micrófono NO procesamos entrada por serial,
    // para no mezclar lectura de comandos con la salida binaria del audio.
    if (micService.isStreaming()) {
        return;
    }

    wifiConnected = (WiFi.status() == WL_CONNECTED);

    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            serialInputBuffer.trim();

            if (serialInputBuffer.length() > 0 && serialInputBuffer.startsWith("{")) {
                processLocalMessage(serialInputBuffer);
            }

            serialInputBuffer = "";
        } else {
            serialInputBuffer += c;

            if (serialInputBuffer.length() > 1024) {
                serialInputBuffer = "";
                Serial.println("{\"type\":\"ERROR\",\"event\":\"SERIAL_INPUT_OVERFLOW\"}");
            }
        }
    }
}

// =============================
// ENVÍO DE EVENTOS
// =============================
void sendHardwareAction(String action, String value) {
    StaticJsonDocument<160> doc;
    doc["type"] = "HW_EVENT";
    doc["action"] = action;
    doc["value"] = value;

    serializeJson(doc, Serial);
    Serial.println();
}

// =============================
// PROCESAMIENTO DE COMANDOS
// =============================
void processLocalMessage(String jsonPayload) {
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, jsonPayload);

    if (error) {
        StaticJsonDocument<160> errDoc;
        errDoc["type"] = "ERROR";
        errDoc["event"] = "JSON_PARSE_FAILED";
        errDoc["detail"] = error.c_str();
        serializeJson(errDoc, Serial);
        Serial.println();
        return;
    }

    String type = doc["type"] | "";

    if (type == "CMD_AUDIO") {
        String file = doc["file"] | "";
        bool loop = doc["loop"] | false;

        if (file.length() > 0) {
            audioService.play(file, loop);
        } else {
            Serial.println("{\"type\":\"ERROR\",\"event\":\"CMD_AUDIO_FILE_EMPTY\"}");
        }
    }
    else if (type == "CMD_LED") {
        int estado = doc["state"] | 0;

        if (estado >= IDLE && estado <= MOVE_RIGHT) {
            ledService.setEstado((LedEstado)estado);
        } else {
            Serial.println("{\"type\":\"ERROR\",\"event\":\"CMD_LED_INVALID_STATE\"}");
        }
    }
    else if (type == "CMD_LED_FLASH") {
        int estado = doc["state"] | 0;

        if (estado >= IDLE && estado <= MOVE_RIGHT) {
            ledService.triggerFlash((LedEstado)estado);
        } else {
            Serial.println("{\"type\":\"ERROR\",\"event\":\"CMD_LED_FLASH_INVALID_STATE\"}");
        }
    }
    else if (type == "CMD_STOP_ALL") {
        audioService.stop();
        micService.stopStreaming();
        ledService.setEstado(IDLE);
    }
    else if (type == "CMD_SEQUENCE") {
        if (!doc["files"].is<JsonArray>()) {
            Serial.println("{\"type\":\"ERROR\",\"event\":\"CMD_SEQUENCE_INVALID_FILES\"}");
            return;
        }

        JsonArray files = doc["files"].as<JsonArray>();
        std::vector<String> sequence;
        sequence.reserve(files.size());

        for (JsonVariant f : files) {
            String name = f.as<String>();
            name.trim();
            if (name.length() > 0) {
                sequence.push_back(name);
            }
        }

        if (!sequence.empty()) {
            audioService.playSequence(sequence);
        } else {
            Serial.println("{\"type\":\"ERROR\",\"event\":\"CMD_SEQUENCE_EMPTY\"}");
        }
    }
    else if (type == "CMD_MIC_START") {
        uint8_t player = doc["player"] | 0;
        micService.startStreaming(player);
    }
    else if (type == "CMD_MIC_STOP") {
        micService.stopStreaming();
    }
    else if (type == "CMD_VOLUME") {
        int value = doc["value"] | 80;
        audioService.setVolumePercentage(value);
    }
    else if (type == "CMD_PING") {
        StaticJsonDocument<192> pong;
        pong["type"] = "SYSTEM";
        pong["event"] = "PONG";
        pong["wifi"] = wifiConnected;
        pong["mic"] = micService.isStreaming();
        pong["audio"] = audioService.isPlaying();
        pong["sd"] = audioService.isSdReady();
        pong["activePlayer"] = micService.getActivePlayer();
        serializeJson(pong, Serial);
        Serial.println();
    }
    else {
        StaticJsonDocument<160> errDoc;
        errDoc["type"] = "ERROR";
        errDoc["event"] = "UNKNOWN_COMMAND";
        errDoc["cmd"] = type;
        serializeJson(errDoc, Serial);
        Serial.println();
    }
}