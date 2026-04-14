#ifndef MIC_SERVICE_H
#define MIC_SERVICE_H

#include <driver/i2s.h>
#include "HardwareMap.h"

// Cambiamos al puerto 0, el puerto 1 lo dejamos para el DAC (AudioService)
#define I2S_MIC_PORT I2S_NUM_0 
#define MIC_BUFFER_SIZE 1024 // Aumentamos buffer para estabilidad

class MicService {
private:
    int16_t buffer[MIC_BUFFER_SIZE]; // Usamos int16_t para coincidir con el micro
    bool _isStreaming;

public:
    MicService();
    void setup();
    void startStreaming(); 
    void stopStreaming();
    void update(); 
    bool isStreaming() { return _isStreaming; }
};

extern MicService micService;

#endif