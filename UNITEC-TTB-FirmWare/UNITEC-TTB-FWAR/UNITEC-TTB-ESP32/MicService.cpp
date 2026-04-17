#include "MicService.h"
#include "AudioService.h"

MicService::MicService()
    : _isStreaming(false),
      _isReady(false),
      _activePlayer(0) {}

void MicService::setup() {
    _isReady = true;
    _isStreaming = false;
    _activePlayer = 0;

    Serial.println("{\"type\":\"SYSTEM\",\"event\":\"MIC_READY\"}");
}

void MicService::startStreaming(uint8_t player) {
    if (!_isReady || _isStreaming) return;
    if (player != 1 && player != 2) return;

    audioService.stop();
    delay(40);

    _activePlayer = player;
    _isStreaming = true;

    Serial.printf("{\"type\":\"SYSTEM\",\"event\":\"MIC_STREAM_STARTED\",\"player\":%u}\n", _activePlayer);
}

void MicService::stopStreaming() {
    if (!_isStreaming) return;

    Serial.printf("{\"type\":\"SYSTEM\",\"event\":\"MIC_STREAM_STOPPED\",\"player\":%u}\n", _activePlayer);

    _isStreaming = false;
    _activePlayer = 0;
}

void MicService::update() {
    // Ya no capturamos audio real desde el ESP.
    // El micrófono de la computadora hará la captura.
}

MicService micService;