#ifndef LED_SERVICE_H
#define LED_SERVICE_H

#include <Adafruit_NeoPixel.h>
#include "HardwareMap.h"

// Definición de estados para mayor claridad en el código
enum LedEstado
{
    IDLE = 0,
    REGISTRO = 1,
    TURNOS = 2,
    CATEGORIA = 3,
    CARTA = 4,
    JUEGO_SIRENA = 5,
    ESPERA_VALIDEZ = 6,
    CORRECTO = 7,
    INCORRECTO = 8,
    FIN_JUEGO = 9
};

class LedService
{
private:
    Adafruit_NeoPixel pixels;
    int estadoActual;
    unsigned long lastUpdate;
    int pixelAnim;

    // Colores predefinidos
    uint32_t colorAzul;
    uint32_t colorRosa;
    uint32_t colorBlanco;
    uint32_t colorVerde;
    uint32_t colorRojo;

public:
    LedService(); // Constructor
    void setup();
    void setEstado(int nuevoEstado);
    void update();
    void clear();
};

// Declaramos la instancia como externa para que main.cpp la vea
extern LedService ledService;

#endif