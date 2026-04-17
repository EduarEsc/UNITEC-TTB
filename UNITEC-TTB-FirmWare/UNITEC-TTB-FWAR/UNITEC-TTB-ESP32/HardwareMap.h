#ifndef HARDWARE_MAP_H
#define HARDWARE_MAP_H

#include <Arduino.h>

// IMPORTANTE:
// 16 kHz * 16 bits * mono = 32 KB/s aprox.
// A 115200 baud NO alcanza.
// 921600 sí da margen suficiente para audio + eventos.
#define SERIAL_BAUD 921600

// =============================
// I2S ENTRADA (Micrófono INMP441)
// =============================
#define I2S_MIC_SD   42  // GPIO 42
#define I2S_MIC_SCK  1   // GPIO 1
#define I2S_MIC_WS   2   // GPIO 2

// =============================
// I2S SALIDA (DAC PCM5102A)
// =============================
#define I2S_DAC_DIN  11  // GPIO 11
#define I2S_DAC_BCK  12  // GPIO 12
#define I2S_DAC_LCK  13  // GPIO 13

// =============================
// PUERTOS I2S
// =============================
#define I2S_DAC_PORT I2S_NUM_0
#define I2S_MIC_PORT I2S_NUM_1

// =============================
// MICRO SD (SPI)
// =============================
#define SD_CS_PIN    10
#define SD_SCK_PIN   15
#define SD_MOSI_PIN  16
#define SD_MISO_PIN  17

// =============================
// NEOPIXEL
// =============================
#define NEOPIXEL_PIN 18
#define NUM_LEDS     16

// =============================
// CONTROLES (activos en LOW)
// =============================
#define BTN_SELECT_PIN    8   // Siguiente / Confirmar
#define BTN_TURNO_PIN     6   // Dado de turnos
#define BTN_CATEGORIA_PIN 7   // Dado de categoría
#define BTN_P1_PIN        14  // Micrófono jugador 1
#define BTN_P2_PIN        21  // Micrófono jugador 2
#define JOY_RIGHT_PIN     4   // Mover derecha
#define JOY_LEFT_PIN      5   // Mover izquierda

void setupHardware();

#endif