#include "HardwareMap.h"

void setupHardware() {
    const uint8_t controlPins[] = {
        BTN_SELECT_PIN,
        BTN_TURNO_PIN,
        BTN_CATEGORIA_PIN,
        BTN_P1_PIN,
        BTN_P2_PIN,
        JOY_RIGHT_PIN,
        JOY_LEFT_PIN
    };

    for (uint8_t i = 0; i < (sizeof(controlPins) / sizeof(controlPins[0])); i++) {
        pinMode(controlPins[i], INPUT_PULLUP);
    }

    // NeoPixel
    pinMode(NEOPIXEL_PIN, OUTPUT);
    digitalWrite(NEOPIXEL_PIN, LOW);

    // SD
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);

    delay(200);

    Serial.begin(SERIAL_BAUD);

    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 3000)) {
        delay(10);
    }

    Serial.println("{\"type\":\"SYSTEM\",\"event\":\"REBOOT_OK\"}");
}