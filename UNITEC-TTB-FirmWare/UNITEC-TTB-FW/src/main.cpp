#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "HardwareMap.h"
#include "AudioService.h"
#include "LedService.h"
#include "MicService.h"
#include "NetworkService.h"

// Estructura para evitar rebotes (Debounce)
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 250;

void setup()
{
  Serial.begin(115200);
  delay(1000); // Tiempo para estabilizar el puerto serie

  // 1. Inicializar Pines (Botones, Joystick, NeoPixel)
  setupHardware();

  // 2. Inicializar Servicios Visuales
  ledService.setup();
  ledService.setEstado(IDLE);

  // 3. Inicializar Audio (Ahora desde LittleFS, no SD)
  audioService.setup();

  // 4. Inicializar Micrófono I2S
  micService.setup();

  // 5. Conectar a WiFi y Backend (WebSocket)
  setupNetwork();

  Serial.println("🚀 Sistema Protocolo Tic-Tac Iniciado");
}

void loop()
{
  // --- MANTENIMIENTO CONSTANTE ---
  updateNetwork();     // Procesa mensajes entrantes de Vue
  audioService.loop(); // Procesa el audio y el volumen dinámico
  ledService.update(); // Procesa las animaciones de los LEDs

  // --- LÓGICA DEL MICRÓFONO ---
  // Si el jugador mantiene presionado su botón "GO", habilitamos el streaming
  bool isP1Talking = (digitalRead(BTN_P1_GO_PIN) == LOW);
  bool isP2Talking = (digitalRead(BTN_P2_GO_PIN) == LOW);

  if (isP1Talking || isP2Talking)
  {
    if (!micService.isStreaming())
    {
      micService.startStreaming();
      ledService.setEstado(ESPERA_VALIDEZ); // Feedback visual blanco
    }
    micService.update(); // Envía los bytes al backend
  }
  else
  {
    if (micService.isStreaming())
    {
      micService.stopStreaming();
      // El estado de LED regresará al anterior mediante el backend o lógica de juego
    }
  }

  // --- LECTURA DE BOTONES Y NAVEGACIÓN (CON DEBOUNCE) ---
  if ((millis() - lastDebounceTime) > debounceDelay)
  {

    // Navegación de Menú
    if (digitalRead(BTN_SELECT_PIN) == LOW)
    {
      sendHardwareAction("BTN_SELECT");
      lastDebounceTime = millis();
    }
    if (digitalRead(BTN_TURN_PIN) == LOW)
    {
      sendHardwareAction("BTN_TURN");
      lastDebounceTime = millis();
    }
    if (digitalRead(BTN_CAT_PIN) == LOW)
    {
      sendHardwareAction("BTN_CAT");
      lastDebounceTime = millis();
    }

    // Joystick (Navegación de cartas/mazo en el Front)
    if (digitalRead(JOY_LEFT_PIN) == LOW)
    {
      sendHardwareAction("JOYSTICK_MOVE", "L");
      lastDebounceTime = millis();
    }
    if (digitalRead(JOY_RIGHT_PIN) == LOW)
    {
      sendHardwareAction("JOYSTICK_MOVE", "R");
      lastDebounceTime = millis();
    }

    // Notificación de clic (Submit)
    // Usamos el flanco de bajada para avisar que un jugador va a responder
    if (isP1Talking)
    {
      sendHardwareAction("BTN_P1_SUBMIT");
      lastDebounceTime = millis();
    }
    if (isP2Talking)
    {
      sendHardwareAction("BTN_P2_SUBMIT");
      lastDebounceTime = millis();
    }
  }
}