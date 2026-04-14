#ifndef MIC_SERVICE_H
#define MIC_SERVICE_H

#include <driver/i2s.h>
#include <WebSocketsClient.h>
#include "HardwareMap.h"

#define I2S_MIC_PORT I2S_NUM_0
#define MIC_BUFFER_SIZE 512

class MicService
{
private:
    uint8_t buffer[MIC_BUFFER_SIZE];
    bool _isStreaming;

public:
    MicService();
    void setup();
    void startStreaming();
    void stopStreaming();
    void update(); // Se llama en el loop principal
    bool isStreaming() { return _isStreaming; }
};

extern MicService micService;

#endif