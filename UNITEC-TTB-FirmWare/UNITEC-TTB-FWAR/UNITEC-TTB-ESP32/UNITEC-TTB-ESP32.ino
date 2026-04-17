#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "HardwareMap.h"
#include "LedService.h"
#include "MicService.h"
#include "NetworkService.h"
#include "AudioService.h"

// =============================
// DEBOUNCE INDIVIDUAL POR ENTRADA
// =============================
unsigned long lastMicToggleTime = 0;
const unsigned long micToggleCooldown = 350;
const unsigned long debounceDelay = 50;

unsigned long lastSelectDebounceTime = 0;
unsigned long lastTurnoDebounceTime  = 0;
unsigned long lastCatDebounceTime    = 0;
unsigned long lastP1DebounceTime     = 0;
unsigned long lastP2DebounceTime     = 0;

bool lastStateSelect = HIGH;
bool lastStateTurno  = HIGH;
bool lastStateCat    = HIGH;
bool lastStateLeft   = HIGH;
bool lastStateRight  = HIGH;
bool lastStateP1     = HIGH;
bool lastStateP2     = HIGH;

// =============================
// CONTROL DE REPETICIÓN PALANCA
// =============================
unsigned long lastMoveTime = 0;
const unsigned long initialDelay = 400;
const unsigned long scrollSpeed  = 200;
bool isFirstMove = true;

// =============================
// MICRÓFONO MODO TOGGLE
// 0 = ninguno, 1 = P1, 2 = P2
// =============================
uint8_t activeMicPlayer = 0;

// =============================
// HELPERS
// =============================
bool isFallingEdge(bool currentState, bool lastState) {
    return (lastState == HIGH && currentState == LOW);
}

void handleButtonSelect() {
    bool currSelect = digitalRead(BTN_SELECT_PIN);

    if (isFallingEdge(currSelect, lastStateSelect)) {
        if (millis() - lastSelectDebounceTime >= debounceDelay) {
            sendHardwareAction("BTN_SELECT");
            ledService.triggerFlash(BTN_SELECT_FLASH);
            lastSelectDebounceTime = millis();
        }
    }

    lastStateSelect = currSelect;
}

void handleButtonTurno() {
    bool currTurno = digitalRead(BTN_TURNO_PIN);

    if (isFallingEdge(currTurno, lastStateTurno)) {
        if (millis() - lastTurnoDebounceTime >= debounceDelay) {
            sendHardwareAction("BTN_TURNO");
            ledService.setEstado(DADO_TURNOS);
            lastTurnoDebounceTime = millis();
        }
    }

    lastStateTurno = currTurno;
}

void handleButtonCategoria() {
    bool currCat = digitalRead(BTN_CATEGORIA_PIN);

    if (isFallingEdge(currCat, lastStateCat)) {
        if (millis() - lastCatDebounceTime >= debounceDelay) {
            sendHardwareAction("BTN_CATEGORIA");
            ledService.setEstado(DADO_CATEGORIA);
            lastCatDebounceTime = millis();
        }
    }

    lastStateCat = currCat;
}

void handleJoystick() {
    unsigned long currentMillis = millis();

    bool currL = digitalRead(JOY_LEFT_PIN);
    bool currR = digitalRead(JOY_RIGHT_PIN);

    if (currL == HIGH && currR == HIGH) {
        isFirstMove = true;
    }

    if (currL == LOW && currR == HIGH) {
        if (lastStateLeft == HIGH) {
            sendHardwareAction("HW_MOVE", "L");
            ledService.triggerFlash(MOVE_LEFT);
            lastMoveTime = currentMillis;
            isFirstMove = true;
        } 
        else if (currentMillis - lastMoveTime >= (isFirstMove ? initialDelay : scrollSpeed)) {
            sendHardwareAction("HW_MOVE", "L");
            ledService.triggerFlash(MOVE_LEFT);
            lastMoveTime = currentMillis;
            isFirstMove = false;
        }
    }
    else if (currR == LOW && currL == HIGH) {
        if (lastStateRight == HIGH) {
            sendHardwareAction("HW_MOVE", "R");
            ledService.triggerFlash(MOVE_RIGHT);
            lastMoveTime = currentMillis;
            isFirstMove = true;
        } 
        else if (currentMillis - lastMoveTime >= (isFirstMove ? initialDelay : scrollSpeed)) {
            sendHardwareAction("HW_MOVE", "R");
            ledService.triggerFlash(MOVE_RIGHT);
            lastMoveTime = currentMillis;
            isFirstMove = false;
        }
    }

    lastStateLeft  = currL;
    lastStateRight = currR;
}

// =============================
// MICRÓFONO TOGGLE
// clic 1 = activar
// clic 2 = desactivar y enviar
// =============================
void handleMicToggle() {
    bool currP1 = digitalRead(BTN_P1_PIN);
    bool currP2 = digitalRead(BTN_P2_PIN);

    // Bloquea toggles muy seguidos
    if (millis() - lastMicToggleTime < micToggleCooldown) {
        lastStateP1 = currP1;
        lastStateP2 = currP2;
        return;
    }

    bool p1Clicked = isFallingEdge(currP1, lastStateP1);
    bool p2Clicked = isFallingEdge(currP2, lastStateP2);

    // --- JUGADOR 1 ---
    if (p1Clicked && (millis() - lastP1DebounceTime >= debounceDelay)) {
        lastP1DebounceTime = millis();

        if (!micService.isStreaming() && activeMicPlayer == 0) {
            activeMicPlayer = 1;
            lastMicToggleTime = millis();   // AQUÍ
            micService.startStreaming(1);
            ledService.setEstado(BTN_PLAYER_FLASH);
        }
        else if (micService.isStreaming() && activeMicPlayer == 1) {
            lastMicToggleTime = millis();   // AQUÍ
            micService.stopStreaming();
            activeMicPlayer = 0;
            ledService.setEstado(VISTA_JUEGO);
        }
    }

    // --- JUGADOR 2 ---
    if (p2Clicked && (millis() - lastP2DebounceTime >= debounceDelay)) {
        lastP2DebounceTime = millis();

        if (!micService.isStreaming() && activeMicPlayer == 0) {
            activeMicPlayer = 2;
            lastMicToggleTime = millis();   // AQUÍ
            micService.startStreaming(2);
            ledService.setEstado(BTN_PLAYER_FLASH);
        }
        else if (micService.isStreaming() && activeMicPlayer == 2) {
            lastMicToggleTime = millis();   // AQUÍ
            micService.stopStreaming();
            activeMicPlayer = 0;
            ledService.setEstado(VISTA_JUEGO);
        }
    }

    lastStateP1 = currP1;
    lastStateP2 = currP2;
}

// =============================
// SETUP
// =============================
void setup() {
    setupHardware();

    ledService.setup();
    audioService.setup();
    micService.setup();

    setupNetwork();

    Serial.println("{\"type\":\"SYSTEM\",\"event\":\"READY_MAIN\"}");
}

// =============================
// LOOP
// =============================
void loop() {
    updateNetwork();
    ledService.update();
    audioService.loop();

    if (micService.isStreaming()) {
        micService.update(); // ahora no hace captura real
        handleMicToggle();
        return;
    }

    handleButtonSelect();
    handleButtonTurno();
    handleButtonCategoria();
    handleJoystick();
    handleMicToggle();
}