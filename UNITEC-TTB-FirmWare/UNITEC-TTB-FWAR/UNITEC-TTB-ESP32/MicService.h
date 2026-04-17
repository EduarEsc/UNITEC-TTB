#ifndef MIC_SERVICE_H
#define MIC_SERVICE_H

#include <Arduino.h>

class MicService {
private:
    bool _isStreaming;
    bool _isReady;
    uint8_t _activePlayer;

public:
    MicService();

    void setup();
    void startStreaming(uint8_t player);
    void stopStreaming();
    void update();

    bool isStreaming() const { return _isStreaming; }
    bool isReady() const { return _isReady; }
    uint8_t getActivePlayer() const { return _activePlayer; }
};

extern MicService micService;

#endif