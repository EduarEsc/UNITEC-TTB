#include "LedService.h"
#include <math.h>

LedService::LedService() : pixels(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800),
                           estadoActual(IDLE), lastUpdate(0), pixelAnim(0), 
                           brilloEfecto(50), subiendoBrillo(true), alternarEstado(false) {}

void LedService::setup() {
    pixels.begin();
    pixels.setBrightness(80); // Un poco más de brillo base
    pixels.clear();
    pixels.show();
}

void LedService::setEstado(int nuevoEstado) {
    // Permitimos reiniciar si es un flash o movimiento de palanca para feedback inmediato
    if (estadoActual == nuevoEstado && 
        nuevoEstado != BTN_SELECT_FLASH && 
        nuevoEstado != MOVE_LEFT && 
        nuevoEstado != MOVE_RIGHT) return; 
    
    estadoActual = nuevoEstado;
    pixelAnim = 0;
    lastUpdate = millis();
    brilloEfecto = 0; 
    alternarEstado = false;
    
    // Limpieza al cambiar de estado
    pixels.setBrightness(100);
    pixels.clear();
    pixels.show();
}

void LedService::update() {
    unsigned long now = millis();
    bool needShow = false;

    switch (estadoActual) {
        
        case VISTA_REGLAS: // Azul y Blanco parpadeo suave
            if (now - lastUpdate > 500) {
                uint32_t color = alternarEstado ? pixels.Color(0,20,200) : pixels.Color(150,150,150);
                for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, color);
                alternarEstado = !alternarEstado;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case VISTA_REGISTRO: // Parpadeo Azul
            if (now - lastUpdate > 300) {
                uint32_t color = alternarEstado ? pixels.Color(0,0,150) : 0;
                for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, color);
                alternarEstado = !alternarEstado;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case DADO_TURNOS:
        case DADO_CATEGORIA: // Giro de fuego mejorado con estela
            if (now - lastUpdate > 40) {
                pixels.clear();
                // Dibujamos el "cometa" (3 leds de diferente intensidad)
                for(int i=0; i<4; i++) {
                    int pos;
                    uint32_t col;
                    if(i == 0) col = pixels.Color(255, 60, 0);   // Cabeza (Naranja/Rojo)
                    else if(i == 1) col = pixels.Color(200, 30, 0); // Cuerpo
                    else col = pixels.Color(100, 0, 0);          // Cola

                    if (estadoActual == DADO_TURNOS)
                        pos = (pixelAnim - i + NUM_LEDS) % NUM_LEDS;
                    else
                        pos = (pixelAnim + i) % NUM_LEDS;

                    pixels.setPixelColor(pos, col);
                }
                pixelAnim = (pixelAnim + 1) % NUM_LEDS;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case SEL_CARTAS: // Respiración Verde Orgánica
            {
                // Uso de función seno para suavidad total
                float intensidad = (sin(now / 500.0) + 1) * 60 + 20; 
                pixels.setBrightness(intensidad);
                for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, pixels.Color(0, 200, 0));
                needShow = true;
            }
            break;

        case VISTA_JUEGO: // Sirena Policía (Rojo vs Azul)
            if (now - lastUpdate > 60) {
                pixels.clear();
                int medio = NUM_LEDS / 2;
                for(int i=0; i<medio; i++) {
                    pixels.setPixelColor((pixelAnim + i) % NUM_LEDS, pixels.Color(200,0,0));
                    pixels.setPixelColor((pixelAnim + i + medio) % NUM_LEDS, pixels.Color(0,0,200));
                }
                pixelAnim = (pixelAnim + 1) % NUM_LEDS;
                lastUpdate = now;
                needShow = true;
            }
            break;

        case MOVE_LEFT: // Feedback Palanca Izquierda (Leds 0-7)
        case MOVE_RIGHT: // Feedback Palanca Derecha (Leds 8-15)
            if (now - lastUpdate < 250) {
                int start = (estadoActual == MOVE_LEFT) ? 0 : NUM_LEDS/2;
                int end = (estadoActual == MOVE_LEFT) ? NUM_LEDS/2 : NUM_LEDS;
                pixels.clear();
                for(int i=start; i<end; i++) pixels.setPixelColor(i, pixels.Color(255, 255, 0)); // Amarillo
                needShow = true;
            } else {
                setEstado(VISTA_REGLAS); // Vuelve al estado base automáticamente
            }
            break;

        case BTN_SELECT_FLASH: // Flash Rosa Feedback
            if (now - lastUpdate < 200) {
                pixels.setBrightness(255);
                for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, pixels.Color(255, 0, 100));
                needShow = true;
            } else {
                setEstado(IDLE); 
            }
            break;

        case BTN_PLAYER_FLASH: // Blanco Fijo (Micrófono activo)
            pixels.setBrightness(200);
            for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, pixels.Color(255, 255, 255));
            needShow = true;
            break;

        case BOMBA_EXPLOTA: // Rojo Sangre pulsante rápido
            {
                int b = 127 + 127 * sin(now / 60.0);
                pixels.setBrightness(b);
                for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, pixels.Color(255, 0, 0));
                needShow = true;
            }
            break;

        case PALABRA_CORRECTA: // Parpadeo Verde Rápido
            if (now - lastUpdate < 1200) {
                bool flash = (now / 100) % 2;
                for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, flash ? pixels.Color(0,255,0) : 0);
                needShow = true;
            } else {
                setEstado(VISTA_JUEGO);
            }
            break;

        case PALABRA_INCORRECTA: // Parpadeo Rojo Rápido
            if (now - lastUpdate < 1200) {
                bool flash = (now / 100) % 2;
                for(int i=0; i<NUM_LEDS; i++) pixels.setPixelColor(i, flash ? pixels.Color(255,0,0) : 0);
                needShow = true;
            } else {
                setEstado(VISTA_JUEGO);
            }
            break;

        case IDLE:
            pixels.clear();
            needShow = true;
            break;
    }

    if (needShow) pixels.show();
}

void LedService::clear() {
    estadoActual = IDLE;
    pixels.clear();
    pixels.show();
}

LedService ledService;