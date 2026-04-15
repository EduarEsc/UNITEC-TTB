#ifndef LED_SERVICE_H
#define LED_SERVICE_H

#include <Adafruit_NeoPixel.h>
#include "HardwareMap.h"

enum LedEstado {
    IDLE = 0,
    VISTA_REGLAS = 1,
    VISTA_REGISTRO = 2,
    DADO_TURNOS = 3,
    DADO_CATEGORIA = 4,
    SEL_CARTAS = 5,
    VISTA_JUEGO = 6,
    BTN_SELECT_FLASH = 7,
    BTN_PLAYER_FLASH = 8,
    BOMBA_EXPLOTA = 9,
    PALABRA_CORRECTA = 10,
    PALABRA_INCORRECTA = 11,
    MOVE_LEFT = 12,
    MOVE_RIGHT = 13
};

class LedService {
private:
    Adafruit_NeoPixel pixels;

    LedEstado estadoBase;
    LedEstado estadoTemporal;
    bool usandoTemporal;

    unsigned long lastUpdate;
    unsigned long effectStartTime;
    int pixelAnim;

    void renderEstado(LedEstado estado, unsigned long now, bool &needShow);
    bool isTemporal(LedEstado estado) const;
    unsigned long getTemporalDuration(LedEstado estado) const;

public:
    LedService();

    void setup();
    void setEstado(LedEstado nuevoEstado);      // Cambia estado base
    void triggerFlash(LedEstado efecto);        // Lanza efecto temporal
    void update();
    void clear();
};

extern LedService ledService;

#endif