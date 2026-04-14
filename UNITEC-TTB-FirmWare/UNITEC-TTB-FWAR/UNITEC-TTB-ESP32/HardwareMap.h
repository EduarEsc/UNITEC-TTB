#ifndef HARDWARE_MAP_H
#define HARDWARE_MAP_H

#include <Arduino.h>

#define SERIAL_BAUD 921600

// --- BUS I2S ENTRADA (Micrófono) ---
#define I2S_MIC_SD   42  // GPIO 42
#define I2S_MIC_SCK  1   // GPIO 01
#define I2S_MIC_WS   2   // GPIO 02

// --- BUS I2S SALIDA (DAC PCM5102A) ---
// Comparten pines con la SD para optimizar el bus
#define I2S_DAC_DIN  11  // GPIO 11
#define I2S_DAC_BCK  12  // GPIO 12
#define I2S_DAC_LCK  13  // GPIO 13

// --- CONFIGURACIÓN DE PUERTOS I2S ---
#define I2S_MIC_PORT I2S_NUM_0  // El micrófono usa el puerto 0
#define I2S_DAC_PORT I2S_NUM_1  // El DAC (audio) usa el puerto 1

// --- MICRO SD CARD (SPI) ---
#define SD_CS_PIN    10  
#define SD_SCK_PIN 15  
#define SD_MOSI_PIN 16
#define SD_MISO_PIN 17

// --- LUCES (NeoPixel) ---
#define NEOPIXEL_PIN 18  
#define NUM_LEDS     16  

// --- CONTROLES (Con Pull-Up Externo de 10k) ---
#define BTN_SELECT_PIN    8   // Actúa como "Siguiente" y "Confirmar"
#define BTN_TURNO_PIN     6   // Gira dado de turnos
#define BTN_CATEGORIA_PIN 7   // Gira dado de categoría
#define BTN_P1_PIN        14  // Activa Mic para Jugador 1
#define BTN_P2_PIN        21  // Activa Mic para Jugador 2
#define JOY_RIGHT_PIN     4   // Desplazar cartas a la derecha
#define JOY_LEFT_PIN      5   // Desplazar cartas a la izquierda

void setupHardware(); 

#endif