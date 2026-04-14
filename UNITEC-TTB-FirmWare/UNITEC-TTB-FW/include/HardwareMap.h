#ifndef HARDWARE_MAP_H
#define HARDWARE_MAP_H

#include <Arduino.h>

// --- BUS I2S COMPARTIDO (Audio & Mic) ---
// Ambos dispositivos comparten Reloj y Word Select según tu conexión
#define I2S_BCLK_SCK 5 // Pin 05: BCLK (Amp) y SCK (Mic)
#define I2S_LRC_WS 4   // Pin 04: LRC (Amp) y WS (Mic)

// --- DATOS SEPARADOS I2S ---
#define I2S_DOUT 6   // Pin 06: DIN del Amplificador
#define I2S_MIC_SD 1 // Pin 01: SD del Micrófono

// --- LUCES (NeoPixel) ---
#define NEOPIXEL_PIN 38 // Pin 38: Data con resistencia 330 ohm
#define NUM_LEDS 12     // Ajusta según los que tenga tu aro/tira

// --- JOYSTICK (Direcciones) ---
#define JOY_RIGHT_PIN 10 // Pin 10: Dirección Derecha (A2)
#define JOY_LEFT_PIN 11  // Pin 11: Dirección Izquierda (B4)

// --- BOTONES DE SISTEMA ---
#define BTN_CAT_PIN 21    // Pin 21: Botón Categoría
#define BTN_TURN_PIN 47   // Pin 47: Botón Turno
#define BTN_SELECT_PIN 48 // Pin 48: Selección Universal

// --- BOTONES DE JUGADOR (Confirmación Física) ---
#define BTN_P1_GO_PIN 14 // Pin 14: Botón P1
#define BTN_P2_GO_PIN 45 // Pin 45: Botón P2

// Prototipo de función para inicializar pines
void setupHardware();

#endif