#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "HardwareMap.h"
#include "AudioService.h"
#include "LedService.h"
#include "MicService.h" // Agregado para control de flujo

void setupNetwork();  
void updateNetwork(); 

// Envío de eventos (Botones, Joystick, etc.)
void sendHardwareAction(String action, String value = "");

// Procesamiento de comandos que vienen de Python (Laptop)
void processLocalMessage(String jsonPayload);

#endif