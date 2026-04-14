#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <vector>
#include "HardwareMap.h"
#include "AudioService.h"
#include "LedService.h"

// Declaraciones externas
extern WebSocketsClient webSocket;

// Funciones principales
void setupNetwork();
void updateNetwork(); // Para llamar en el loop principal
void sendHardwareAction(String action, String direction = "");

// Manejador de eventos (interno del .cpp)
void onWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);

#endif