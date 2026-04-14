#include "HardwareMap.h"

void setupHardware() {
    // Lista de pines de control
    const uint8_t controlPins[] = {
        BTN_SELECT_PIN,    // 8
        BTN_TURNO_PIN,     // 6
        BTN_CATEGORIA_PIN, // 7
        BTN_P1_PIN,        // 14
        BTN_P2_PIN,        // 21
        JOY_RIGHT_PIN,     // 4
        JOY_LEFT_PIN       // 5  <-- Asegúrate que el cable esté en el GPIO 5
    };

    // Usaremos INPUT_PULLUP por seguridad. 
    // Si tu resistencia externa es de 10k y está a 3.3V, esto reforzará la señal.
    for (uint8_t i = 0; i < sizeof(controlPins); i++) {
        pinMode(controlPins[i], INPUT_PULLUP); 
    }

    // Configuración de periféricos de salida
    pinMode(NEOPIXEL_PIN, OUTPUT);
    digitalWrite(NEOPIXEL_PIN, LOW); // Apagar tira al iniciar
    
    // Pin CS de la SD
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH); // Desactivar SD inicialmente

    // El ESP32-S3 a veces necesita un poco de tiempo para estabilizar voltajes
    delay(200);

    Serial.begin(115200);
    
    // Espera un máximo de 3 segundos al USB (para no trabarse si juegas con batería)
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 3000)) {
        delay(10); 
    }

    // Mensaje de sistema para el Backend
    Serial.println("{\"type\":\"SYSTEM\", \"event\":\"REBOOT_OK\"}");
}