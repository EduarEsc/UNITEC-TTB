#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#include "Audio.h"      
#include "HardwareMap.h"
#include <vector>        
#include <SD.h>         
#include <WiFi.h>

class AudioService {
private:
    // Forzamos el uso del Puerto I2S 1 para el DAC
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

    void handleAudioEOF();          
};

extern AudioService audioService;

#endif