#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#include <Arduino.h>
#include <vector>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include "Audio.h"
#include "HardwareMap.h"

class AudioService {
private:
    Audio audio;
    std::vector<String> audioQueue;

    bool sdReady;
    bool isSequencePlaying;
    bool isIncreasingVol;

    unsigned long lastVolUpdate;
    int currentVol;

    String serverBaseUrl;

    bool playFromSD(const String& audioId);
    bool playFromWiFi(const String& audioId);
    bool playInternal(const String& audioId);

public:
    bool isLooping;
    String currentLoopFile;

    AudioService();

    void setup();
    void loop();

    bool isReady() const;
    bool isPlaying();
    bool isSdReady() const;

    void setServerBaseUrl(const String& baseUrl);

    void play(String audioId, bool loop = false);
    void playExplosion();
    void playSequence(const std::vector<String>& ids);
    void stop();

    void setVolumePercentage(int percentage);
    void startIncreasingVolume();

    void handleAudioEOF();
};

extern AudioService audioService;

#endif