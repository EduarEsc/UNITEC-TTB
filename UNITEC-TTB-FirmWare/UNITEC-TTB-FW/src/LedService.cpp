#include "LedService.h"

// Constructor: Inicializamos el objeto pixels con los pines del HardwareMap
LedService::LedService() : pixels(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800),
                           estadoActual(0),
                           lastUpdate(0),
                           pixelAnim(0)
{
}

void LedService::setup()
{
    pixels.begin();
    pixels.setBrightness(50); // Brillo inicial moderado
    pixels.show();

    // Inicializar colores después de pixels.begin()
    colorAzul = pixels.Color(0, 0, 255);
    colorRosa = pixels.Color(255, 20, 147);
    colorBlanco = pixels.Color(255, 255, 255);
    colorVerde = pixels.Color(0, 255, 0);
    colorRojo = pixels.Color(255, 0, 0);
}

void LedService::setEstado(int nuevoEstado)
{
    if (estadoActual == nuevoEstado)
        return;

    estadoActual = nuevoEstado;
    pixelAnim = 0;
    lastUpdate = 0;

    // Resetear brillo si salimos de estados intensos
    if (nuevoEstado != CORRECTO && nuevoEstado != INCORRECTO)
    {
        pixels.setBrightness(50);
    }

    pixels.clear();
    pixels.show();
}

void LedService::update()
{
    switch (estadoActual)
    {
    case REGISTRO: // Parpadeo Azul
        if (millis() - lastUpdate > 500)
        {
            static bool on = false;
            for (int i = 0; i < NUM_LEDS; i++)
                pixels.setPixelColor(i, on ? colorAzul : 0);
            on = !on;
            lastUpdate = millis();
        }
        break;

    case TURNOS: // Giro Derecha Azul
        if (millis() - lastUpdate > 80)
        {
            pixels.clear();
            pixels.setPixelColor(pixelAnim, colorAzul);
            pixelAnim = (pixelAnim + 1) % NUM_LEDS;
            lastUpdate = millis();
        }
        break;

    case CATEGORIA: // Giro Izquierda Azul
        if (millis() - lastUpdate > 80)
        {
            pixels.clear();
            pixels.setPixelColor(pixelAnim, colorAzul);
            pixelAnim = (pixelAnim - 1 + NUM_LEDS) % NUM_LEDS;
            lastUpdate = millis();
        }
        break;

    case CARTA: // Estático Rosa
        for (int i = 0; i < NUM_LEDS; i++)
            pixels.setPixelColor(i, colorRosa);
        break;

    case JUEGO_SIRENA: // Alerta: Rojo, Naranja, Amarillo
        if (millis() - lastUpdate > 150)
        {
            static int colorIdx = 0;
            uint32_t colores[] = {pixels.Color(255, 0, 0), pixels.Color(255, 100, 0), pixels.Color(255, 200, 0)};
            for (int i = 0; i < NUM_LEDS; i++)
                pixels.setPixelColor(i, colores[colorIdx]);
            colorIdx = (colorIdx + 1) % 3;
            lastUpdate = millis();
        }
        break;

    case ESPERA_VALIDEZ: // Blanco fijo
        for (int i = 0; i < NUM_LEDS; i++)
            pixels.setPixelColor(i, colorBlanco);
        break;

    case CORRECTO: // Flash Verde
        pixels.setBrightness(255);
        for (int i = 0; i < NUM_LEDS; i++)
            pixels.setPixelColor(i, colorVerde);
        break;

    case INCORRECTO: // Flash Rojo
        pixels.setBrightness(255);
        for (int i = 0; i < NUM_LEDS; i++)
            pixels.setPixelColor(i, colorRojo);
        break;

    case FIN_JUEGO: // Arcoiris de victoria
        if (millis() - lastUpdate > 200)
        {
            static int cycle = 0;
            uint32_t finColores[] = {colorAzul, colorBlanco, colorVerde};
            for (int i = 0; i < NUM_LEDS; i++)
                pixels.setPixelColor(i, finColores[cycle]);
            cycle = (cycle + 1) % 3;
            lastUpdate = millis();
        }
        break;

    default:
        pixels.clear();
        break;
    }
    pixels.show();
}

// Creamos la instancia única aquí
LedService ledService;