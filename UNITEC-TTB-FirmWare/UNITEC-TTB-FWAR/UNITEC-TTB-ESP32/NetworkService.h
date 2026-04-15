#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

#include "HardwareMap.h"
#include "AudioService.h"
#include "LedService.h"
#include "MicService.h"

void setupNetwork();
void updateNetwork();

// ESP32 -> Backend Python
void sendHardwareAction(String action, String value = "");

// Python -> ESP32
void processLocalMessage(String jsonPayload);

#endif