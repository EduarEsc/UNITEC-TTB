#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "HardwareMap.h"
#include "LedService.h"
#include "MicService.h"
#include "NetworkService.h"
#include "AudioService.h"

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150; // Reducido un poco para mayor agilidad

bool lastStateSelect = HIGH;
bool lastStateTurno  = HIGH;
bool lastStateCat    = HIGH;
bool lastStateLeft   = HIGH;
bool lastStateRight  = HIGH;

unsigned long lastMoveTime = 0;
const int initialDelay = 400; // Tiempo que espera antes de empezar a hacer scroll
const int scrollSpeed = 200;  // Velocidad de movimiento cuando se queda presionada
bool isFirstMove = true;      // Para diferenciar el primer toque del scroll continuo

void setup() {
    // 1. Configuración física de pines y Serial
    setupHardware(); 
    
    // 2. Iniciar comunicación (WiFi/JSON)
    setupNetwork(); 

    // 3. Iniciar servicios
    audioService.setup();
    ledService.setup(); 
    //micService.setup(); 

    Serial.println("{\"type\":\"SYSTEM\", \"status\":\"READY_V3_S3_WIFI_MODE\"}");
}

void loop() {
    // --- 1. ACTUALIZACIÓN DE SERVICIOS ---
    updateNetwork();   
    ledService.update();  
    audioService.loop(); 
    
    // Solo actualizamos el Mic si está en modo streaming (ahorra CPU)
    if (micService.isStreaming()) {
        micService.update(); 
    }

    // --- 2. LÓGICA DE NAVEGACIÓN (Botones y Palanca) ---
    if ((millis() - lastDebounceTime) > debounceDelay) {
        
        // SELECT: Confirmar
        bool currSelect = digitalRead(BTN_SELECT_PIN);
        if (currSelect == LOW && lastStateSelect == HIGH) {
            sendHardwareAction("BTN_SELECT");
            ledService.setEstado(BTN_SELECT_FLASH); 
            lastDebounceTime = millis();
        }
        lastStateSelect = currSelect;

        // TURNO: Giro Dado
        bool currTurno = digitalRead(BTN_TURNO_PIN);
        if (currTurno == LOW && lastStateTurno == HIGH) {
            sendHardwareAction("BTN_TURNO");
            ledService.setEstado(DADO_TURNOS); // Feedback visual inmediato
            lastDebounceTime = millis();
        }
        lastStateTurno = currTurno;

        // CATEGORIA: Giro Categoría
        bool currCat = digitalRead(BTN_CATEGORIA_PIN);
        if (currCat == LOW && lastStateCat == HIGH) {
            sendHardwareAction("BTN_CATEGORIA");
            ledService.setEstado(DADO_CATEGORIA); // Feedback visual inmediato
            lastDebounceTime = millis();
        }
        lastStateCat = currCat;

        // MOVIMIENTO DE PALANCA
        unsigned long currentMillis = millis();

        // --- LÓGICA PALANCA IZQUIERDA ---
        bool currL = digitalRead(JOY_LEFT_PIN);
        if (currL == LOW) { // Está activada la izquierda
            if (lastStateLeft == HIGH || (currentMillis - lastMoveTime > (isFirstMove ? initialDelay : scrollSpeed))) {
                sendHardwareAction("HW_MOVE", "L");
                ledService.setEstado(MOVE_LEFT);
                
                lastMoveTime = currentMillis;
                isFirstMove = (lastStateLeft == HIGH) ? true : false; 
                if(lastStateLeft == HIGH) isFirstMove = true; // Fue el primer toque
                else isFirstMove = false; // Ya entró en modo scroll
            }
        }

        // --- LÓGICA PALANCA DERECHA ---
        bool currR = digitalRead(JOY_RIGHT_PIN);
        if (currR == LOW) { // Está activada la derecha
            if (lastStateRight == HIGH || (currentMillis - lastMoveTime > (isFirstMove ? initialDelay : scrollSpeed))) {
                sendHardwareAction("HW_MOVE", "R");
                ledService.setEstado(MOVE_RIGHT);
                
                lastMoveTime = currentMillis;
                if(lastStateRight == HIGH) isFirstMove = true;
                else isFirstMove = false;
            }
        }

        // Resetear estados cuando se suelta la palanca
        if (currL == HIGH && currR == HIGH) {
            isFirstMove = true;
        }

        lastStateLeft = currL;
        lastStateRight = currR;
    }

    // --- 3. LÓGICA PUSH-TO-TALK (Prioridad Máxima) ---
    bool p1Presionado = (digitalRead(BTN_P1_PIN) == LOW);
    bool p2Presionado = (digitalRead(BTN_P2_PIN) == LOW);

    if (p1Presionado || p2Presionado) { 
        if (!micService.isStreaming()) {
            // El propio MicService.startStreaming() ya llama a audioService.stop()
            micService.startStreaming();
            ledService.setEstado(BTN_PLAYER_FLASH); // Leds en blanco para indicar escucha
            
            String player = p1Presionado ? "P1" : "P2";
            sendHardwareAction("BTN_TALK_START", player);
        }
    } 
    else {
        // Si se suelta el botón
        if (micService.isStreaming()) {
            micService.stopStreaming();
            sendHardwareAction("BTN_TALK_STOP");
            // Nota: El LED volverá a su estado anterior o IDLE según el comando que mande el Back
        }
    }
}