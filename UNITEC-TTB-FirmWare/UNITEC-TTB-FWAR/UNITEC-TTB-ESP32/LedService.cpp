#include "LedService.h"
#include <math.h>

LedService::LedService()
    : pixels(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800),
      estadoBase(IDLE),
      estadoTemporal(IDLE),
      usandoTemporal(false),
      lastUpdate(0),
      effectStartTime(0),
      pixelAnim(0) {}

void LedService::setup() {
    pixels.begin();
    pixels.setBrightness(80);
    pixels.clear();
    pixels.show();
}

bool LedService::isTemporal(LedEstado estado) const {
    switch (estado) {
        case BTN_SELECT_FLASH:
        case BTN_PLAYER_FLASH:
        case BOMBA_EXPLOTA:
        case PALABRA_CORRECTA:
        case PALABRA_INCORRECTA:
        case MOVE_LEFT:
        case MOVE_RIGHT:
            return true;
        default:
            return false;
    }
}

unsigned long LedService::getTemporalDuration(LedEstado estado) const {
    switch (estado) {
        case BTN_SELECT_FLASH:     return 200;
        case MOVE_LEFT:            return 250;
        case MOVE_RIGHT:           return 250;
        case PALABRA_CORRECTA:     return 1200;
        case PALABRA_INCORRECTA:   return 1200;
        case BOMBA_EXPLOTA:        return 1500;
        case BTN_PLAYER_FLASH:     return 0;    // mientras esté activo manualmente
        default:                   return 0;
    }
}

void LedService::setEstado(LedEstado nuevoEstado) {
    if (isTemporal(nuevoEstado)) {
        triggerFlash(nuevoEstado);
        return;
    }

    if (estadoBase == nuevoEstado && !usandoTemporal) {
        return;
    }

    estadoBase = nuevoEstado;
    pixelAnim = 0;
    lastUpdate = 0;

    if (!usandoTemporal) {
        pixels.clear();
        pixels.show();
    }
}

void LedService::triggerFlash(LedEstado efecto) {
    estadoTemporal = efecto;
    usandoTemporal = true;
    effectStartTime = millis();
    pixelAnim = 0;
    lastUpdate = 0;
}

void LedService::renderEstado(LedEstado estado, unsigned long now, bool &needShow) {
    switch (estado) {

        case VISTA_REGLAS:
            if (now - lastUpdate >= 500) {
                static bool alternar = false;
                uint32_t color = alternar ? pixels.Color(0, 20, 200) : pixels.Color(150, 150, 150);
                pixels.setBrightness(100);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, color);
                }
                alternar = !alternar;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case VISTA_REGISTRO:
            if (now - lastUpdate >= 300) {
                static bool alternar = false;
                uint32_t color = alternar ? pixels.Color(0, 0, 150) : 0;
                pixels.setBrightness(100);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, color);
                }
                alternar = !alternar;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case DADO_TURNOS:
        case DADO_CATEGORIA:
            if (now - lastUpdate >= 40) {
                pixels.clear();
                pixels.setBrightness(120);

                for (int i = 0; i < 4; i++) {
                    int pos;
                    uint32_t col;

                    if (i == 0) col = pixels.Color(255, 60, 0);
                    else if (i == 1) col = pixels.Color(200, 30, 0);
                    else col = pixels.Color(100, 0, 0);

                    if (estado == DADO_TURNOS) {
                        pos = (pixelAnim - i + NUM_LEDS) % NUM_LEDS;
                    } else {
                        pos = (pixelAnim + i) % NUM_LEDS;
                    }

                    pixels.setPixelColor(pos, col);
                }

                pixelAnim = (pixelAnim + 1) % NUM_LEDS;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case SEL_CARTAS:
            if (now - lastUpdate >= 30) {
                float intensidad = (sin(now / 500.0f) + 1.0f) * 60.0f + 20.0f;
                pixels.setBrightness((uint8_t)intensidad);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, pixels.Color(0, 200, 0));
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case VISTA_JUEGO:
            if (now - lastUpdate >= 60) {
                pixels.clear();
                pixels.setBrightness(100);

                int medio = NUM_LEDS / 2;
                for (int i = 0; i < medio; i++) {
                    pixels.setPixelColor((pixelAnim + i) % NUM_LEDS, pixels.Color(200, 0, 0));
                    pixels.setPixelColor((pixelAnim + i + medio) % NUM_LEDS, pixels.Color(0, 0, 200));
                }

                pixelAnim = (pixelAnim + 1) % NUM_LEDS;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case BTN_SELECT_FLASH:
            if (now - lastUpdate >= 30) {
                pixels.setBrightness(255);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, pixels.Color(255, 0, 100));
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case BTN_PLAYER_FLASH:
            if (now - lastUpdate >= 30) {
                pixels.setBrightness(200);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case BOMBA_EXPLOTA:
            if (now - lastUpdate >= 30) {
                int b = (int)(127 + 127 * sin(now / 60.0f));
                if (b < 0) b = 0;
                if (b > 255) b = 255;

                pixels.setBrightness((uint8_t)b);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case PALABRA_CORRECTA:
            if (now - lastUpdate >= 80) {
                bool flash = ((now / 100) % 2) != 0;
                pixels.setBrightness(180);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, flash ? pixels.Color(0, 255, 0) : 0);
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case PALABRA_INCORRECTA:
            if (now - lastUpdate >= 80) {
                bool flash = ((now / 100) % 2) != 0;
                pixels.setBrightness(180);
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, flash ? pixels.Color(255, 0, 0) : 0);
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case MOVE_LEFT:
            if (now - lastUpdate >= 30) {
                pixels.clear();
                pixels.setBrightness(160);
                for (int i = 0; i < NUM_LEDS / 2; i++) {
                    pixels.setPixelColor(i, pixels.Color(255, 255, 0));
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case MOVE_RIGHT:
            if (now - lastUpdate >= 30) {
                pixels.clear();
                pixels.setBrightness(160);
                for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, pixels.Color(255, 255, 0));
                }
                lastUpdate = now;
                needShow = true;
            }
            break;

        case IDLE:
        default:
            if (now - lastUpdate >= 250) {
                pixels.clear();
                pixels.setBrightness(80);
                lastUpdate = now;
                needShow = true;
            }
            break;
    }
}

void LedService::update() {
    unsigned long now = millis();
    bool needShow = false;

    LedEstado estadoAmostrar = usandoTemporal ? estadoTemporal : estadoBase;

    renderEstado(estadoAmostrar, now, needShow);

    if (usandoTemporal) {
        unsigned long duration = getTemporalDuration(estadoTemporal);

        if (duration > 0 && (now - effectStartTime >= duration)) {
            usandoTemporal = false;
            estadoTemporal = IDLE;
            pixelAnim = 0;
            lastUpdate = 0;
        }
    }

    if (needShow) {
        pixels.show();
    }
}

void LedService::clear() {
    estadoBase = IDLE;
    estadoTemporal = IDLE;
    usandoTemporal = false;
    pixelAnim = 0;
    lastUpdate = 0;

    pixels.clear();
    pixels.show();
}

LedService ledService;