#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#include "Audio.h" // La 'A' debe ser mayúscula
#include "HardwareMap.h"
#include <LittleFS.h> // Cambio clave: Usamos la Flash interna
#include <vector>

class AudioService
{
private:
    Audio audio;
    std::vector<String> audioQueue;
    bool isSequencePlaying;
    unsigned long lastVolUpdate;
    int currentVol;
    bool isIncreasingVol;

public:
    bool isLooping;
    String currentLoopFile;

    AudioService();
    void setup();
    void loop();
    void play(String audioId, bool loop = false);
    void playExplosion();
    void playSequence(std::vector<String> ids);
    void stop();
    void setVolumePercentage(int percentage);
    void startIncreasingVolume();

    // Getters para el callback
    bool getIsSequencePlaying() { return isSequencePlaying; }
    std::vector<String> &getAudioQueue() { return audioQueue; }
};

extern AudioService audioService;

#endif