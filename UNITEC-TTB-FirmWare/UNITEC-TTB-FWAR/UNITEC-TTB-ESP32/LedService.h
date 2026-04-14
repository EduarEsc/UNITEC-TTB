#ifndef LED_SERVICE_H
#define LED_SERVICE_H

#include <Adafruit_NeoPixel.h>
#include "HardwareMap.h"

enum LedEstado {
    IDLE = 0,
    VISTA_REGLAS = 1,       // Azul/Blanco parpadeo
    VISTA_REGISTRO = 2,     // Azul parpadeo
    DADO_TURNOS = 3,        // Giro Derecha (Fuego)
    DADO_CATEGORIA = 4,     // Giro Izquierda (Fuego)
    SEL_CARTAS = 5,         // Verde degradado respiración
    VISTA_JUEGO = 6,        // Sirena Policía (Modo espera de palabra)
    BTN_SELECT_FLASH = 7,   // Rosa intenso (Feedback de confirmación)
    BTN_PLAYER_FLASH = 8,   // Blanco fijo (Micrófono activo / Hablando)
    BOMBA_EXPLOTA = 9,      // Rojo palpitante rápido
    PALABRA_CORRECTA = 10,  // Verde parpadeo rápido
    PALABRA_INCORRECTA = 11, // Rojo parpadeo rápido
    MOVE_LEFT = 12,        // Flash lado izquierdo
    MOVE_RIGHT = 13        // Flash lado derecho
};

class LedService {
private:
    Adafruit_NeoPixel pixels;
    int estadoActual;
    unsigned long lastUpdate;
    int pixelAnim;
    int brilloEfecto;
    bool subiendoBrillo;
    bool alternarEstado; // Para parpadeos

public:
    LedService(); 
    void setup();
    void setEstado(int nuevoEstado);
    void update();
    void clear();
};

extern LedService ledService;

#endif