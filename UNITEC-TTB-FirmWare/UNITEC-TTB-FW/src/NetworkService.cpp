#include "NetworkService.h"

// --- CONFIGURACIÓN DE RED ---
const char *ssid = "Galaxy A12";
const char *password = "eduar2804";
const char *server_ip = "10.20.129.114"; // Reemplaza con la IP de tu PC
const uint16_t server_port = 8000;

WebSocketsClient webSocket;

void setupNetwork()
{
    Serial.print("Conectando a WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi Conectado");
    Serial.print("IP del ESP32: ");
    Serial.println(WiFi.localIP());

    // Configuración del WebSocket
    webSocket.begin(server_ip, server_port, "/ws/ESP32");
    webSocket.onEvent(onWebSocketEvent);
    webSocket.setReconnectInterval(5000); // Reintento cada 5 seg
}

void updateNetwork()
{
    webSocket.loop();
}

void sendHardwareAction(String action, String direction) {
    StaticJsonDocument<256> doc; // En V6 necesitas especificar el tamaño
    doc["action"] = action;
    if (direction != "") doc["dir"] = direction;

    String output;
    serializeJson(doc, output);
    webSocket.sendTXT(output);
    Serial.println("📤 Enviado al Back: " + output);
}

void onWebSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.println("❌ Desconectado del Backend");
        break;
    case WStype_CONNECTED:
        Serial.println("✅ Conectado al Backend");
        // Notificar al back que el hardware está listo
        sendHardwareAction("HARDWARE_READY");
        break;
    case WStype_TEXT:
    {
        Serial.printf("📥 Mensaje: %s\n", payload);
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
            return;

        String msgType = doc["type"];

        if (msgType == "PLAY")
        {
            String audioId = doc["id"].as<String>();
            audioService.play(audioId);

            // Mapeo dinámico de LEDs según el audio solicitado
            if (audioId == "1_bienvenida")
                ledService.setEstado(REGISTRO);
            else if (audioId == "2_dado_turno")
                ledService.setEstado(TURNOS);
            else if (audioId == "3_dado_carta")
                ledService.setEstado(CATEGORIA);
            else if (audioId == "4_seleccion")
                ledService.setEstado(CARTA);
            else if (audioId == "6_ganador_duelo")
                ledService.setEstado(FIN_JUEGO);
        }
        else if (msgType == "START_TIC_TAC")
        {
            audioService.startIncreasingVolume();
            audioService.play("5_0_0_tic_tac", true); // Loop
            ledService.setEstado(JUEGO_SIRENA);
        }
        else if (msgType == "BOOM")
        {
            audioService.playExplosion();
            ledService.setEstado(INCORRECTO);
        }
        else if (msgType == "PLAY_SEQ")
        {
            JsonArray idsArr = doc["ids"];
            std::vector<String> ids;
            for (JsonVariant v : idsArr)
                ids.push_back(v.as<String>());

            audioService.playSequence(ids);

            // Feedback visual basado en el primer audio de la secuencia
            if (ids.size() > 0)
            {
                if (ids[0] == "5_1_0_correct")
                    ledService.setEstado(CORRECTO);
                else if (ids[0] == "5_2_0_incorrect")
                    ledService.setEstado(INCORRECTO);
            }
        }
        else if (msgType == "STOP_LOOP")
        {
            audioService.stop();
            ledService.setEstado(IDLE);
        }
        else if (msgType == "SET_LED_STATE")
        {
            ledService.setEstado(doc["estado"].as<int>());
        }
        break;
    }
    default:
        break;
    }
}