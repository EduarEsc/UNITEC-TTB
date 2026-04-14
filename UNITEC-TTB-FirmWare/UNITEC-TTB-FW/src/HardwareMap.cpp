#include "HardwareMap.h"

void setupHardware()
{
    // Configuración de Botones con resistencia Pull-Up interna
    // Esto significa que el pin leerá LOW cuando presiones el botón
    pinMode(BTN_CAT_PIN, INPUT_PULLUP);
    pinMode(BTN_TURN_PIN, INPUT_PULLUP);
    pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
    pinMode(BTN_P1_GO_PIN, INPUT_PULLUP);
    pinMode(BTN_P2_GO_PIN, INPUT_PULLUP);

    // Configuración de Joystick
    pinMode(JOY_LEFT_PIN, INPUT_PULLUP);
    pinMode(JOY_RIGHT_PIN, INPUT_PULLUP);

    // Pin de NeoPixel como salida
    pinMode(NEOPIXEL_PIN, OUTPUT);

    // Los pines I2S (4, 5, 6, 1) se configuran automáticamente
    // dentro de AudioService y MicService al iniciar el bus I2S.

    Serial.println("✅ Hardware Map inicializado correctamente.");
}