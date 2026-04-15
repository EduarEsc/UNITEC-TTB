#ifndef MIC_SERVICE_H
#define MIC_SERVICE_H

#include <Arduino.h>
#include "driver/i2s.h"
#include "HardwareMap.h"

#define MIC_RAW_SAMPLES 512

class MicService {
private:
    int32_t rawBuffer[MIC_RAW_SAMPLES];
    int16_t pcmBuffer[MIC_RAW_SAMPLES];
    bool _isStreaming;
    bool _isReady;

public:
    MicService();

    void setup();
    void startStreaming();
    void stopStreaming();
    void update();

    bool isStreaming() const { return _isStreaming; }
    bool isReady() const { return _isReady; }
};

extern MicService micService;

#endif