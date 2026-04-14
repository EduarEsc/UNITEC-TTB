#include "AudioService.h"

AudioService::AudioService() : isSequencePlaying(false), lastVolUpdate(0),
                               currentVol(12), isIncreasingVol(false), isLooping(false) {}

void AudioService::setup() {
    Serial.println("{\"type\":\"SYSTEM\", \"msg\":\"SD_BYPASS_ACTIVE\"}");

    // Espera para que el WiFi se estabilice antes de abrir el bus I2S
    delay(1000); 

    // Configuramos el DAC en el puerto 1
    // La librería audioI2S usa internamente el puerto que le asignes o el por defecto
    audio.setPinout(I2S_DAC_BCK, I2S_DAC_LCK, I2S_DAC_DIN);
    audio.setVolume(20); 
    audio.forceMono(true); 

    // Limpiamos el buffer del puerto asignado en HardwareMap
    i2s_zero_dma_buffer(I2S_NUM_1); 
    
    Serial.println("{\"type\":\"SYSTEM\", \"status\":\"AUDIO_READY_WIFI\"}");
}

void AudioService::play(String audioId, bool loop) {
    isLooping = loop;
    currentLoopFile = audioId;
    audioId.trim();

    // WiFi Directo (Asegúrate que esta IP sea la de tu laptop)
    if (WiFi.status() == WL_CONNECTED) {
        String url = "http://192.168.1.70:8000/audios/" + audioId + ".mp3";
        
        if(audio.isRunning()) audio.stopSong();
        audio.connecttohost(url.c_str());
        Serial.printf("{\"type\":\"DEBUG\", \"msg\":\"Solicitando WiFi: %s\"}\n", url.c_str());
    } else {
        Serial.println("{\"type\":\"ERROR\", \"msg\":\"WIFI_DISCONNECTED\"}");
    }
}

void AudioService::setVolumePercentage(int percentage) {
    currentVol = map(constrain(percentage, 0, 100), 0, 100, 0, 21);
    audio.setVolume(currentVol);
}

void AudioService::startIncreasingVolume() {
    stop(); 
    setVolumePercentage(20); 
    isIncreasingVol = true;
    lastVolUpdate = millis();
    play("5_0_0_tic_tac", true); 
}

void AudioService::playExplosion() {
    stop();
    setVolumePercentage(100); 
    play("5_0_1_explosion", false);
}

void AudioService::playSequence(std::vector<String> ids) {
    stop(); 
    audioQueue = ids;
    isSequencePlaying = true;
    
    if (!audioQueue.empty()) {
        String first = audioQueue[0];
        audioQueue.erase(audioQueue.begin());
        setVolumePercentage(80); 
        play(first);
    }
}

void AudioService::stop() {
    audio.stopSong();
    isSequencePlaying = false;
    isIncreasingVol = false;
    isLooping = false;
    audioQueue.clear();
}

void AudioService::loop() {
    audio.loop();

    if (isIncreasingVol && audio.isRunning()) {
        if (millis() - lastVolUpdate > 450) { 
            if (currentVol < 21) {
                currentVol++;
                audio.setVolume(currentVol);
            } else {
                isIncreasingVol = false; 
            }
            lastVolUpdate = millis();
        }
    }
}

void AudioService::handleAudioEOF() {
    if (isLooping) {
        play(currentLoopFile, true); 
    } 
    else if (isSequencePlaying && !audioQueue.empty()) {
        String next = audioQueue[0];
        audioQueue.erase(audioQueue.begin());
        play(next);
    } else {
        isSequencePlaying = false;
    }
}

AudioService audioService;

void audio_eof_mp3(const char *info) {
    audioService.handleAudioEOF();
}