#include "AudioService.h"

static SPIClass sdSpi(FSPI);

AudioService::AudioService()
    : sdReady(false),
      isSequencePlaying(false),
      isIncreasingVol(false),
      lastVolUpdate(0),
      currentVol(14),
      serverBaseUrl(""),
      isLooping(false),
      currentLoopFile("") {}

bool AudioService::isReady() const {
    return sdReady || WiFi.status() == WL_CONNECTED;
}

bool AudioService::isPlaying() {
    return audio.isRunning();
}

bool AudioService::isSdReady() const {
    return sdReady;
}

void AudioService::setServerBaseUrl(const String& baseUrl) {
    serverBaseUrl = baseUrl;
}

void AudioService::setup() {
    Serial.println("{\"type\":\"SYSTEM\",\"event\":\"AUDIO_INIT_START\"}");

    sdSpi.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    if (!SD.begin(SD_CS_PIN, sdSpi)) {
        sdReady = false;
        Serial.println("{\"type\":\"WARN\",\"event\":\"SD_NOT_READY\"}");
    } else {
        sdReady = true;
        Serial.println("{\"type\":\"SYSTEM\",\"event\":\"SD_INIT_OK\"}");
    }

    audio.setPinout(I2S_DAC_BCK, I2S_DAC_LCK, I2S_DAC_DIN);

    // Menos agresivo para evitar distorsión
    audio.setVolume(currentVol);

    // Importante: no forzar mono mientras estabilizamos salida
    // audio.forceMono(true);

    Serial.println("{\"type\":\"SYSTEM\",\"event\":\"AUDIO_READY\"}");
}

bool AudioService::playFromSD(const String& audioId) {
    if (!sdReady) {
        Serial.println("{\"type\":\"WARN\",\"event\":\"SD_NOT_READY\"}");
        return false;
    }

    String path = "/" + audioId + ".mp3";

    if (!SD.exists(path.c_str())) {
        Serial.printf("{\"type\":\"WARN\",\"event\":\"SD_FILE_NOT_FOUND\",\"file\":\"%s\"}\n", path.c_str());
        return false;
    }

    if (audio.isRunning()) {
        audio.stopSong();
        delay(60);
    }

    bool ok = audio.connecttoFS(SD, path.c_str());

    if (ok) {
        Serial.printf("{\"type\":\"SYSTEM\",\"event\":\"AUDIO_PLAY_SD\",\"file\":\"%s\"}\n", path.c_str());
    } else {
        Serial.printf("{\"type\":\"ERROR\",\"event\":\"AUDIO_PLAY_SD_FAILED\",\"file\":\"%s\"}\n", path.c_str());
    }

    return ok;
}

bool AudioService::playFromWiFi(const String& audioId) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("{\"type\":\"WARN\",\"event\":\"WIFI_NOT_CONNECTED_FOR_AUDIO\"}");
        return false;
    }

    if (serverBaseUrl.length() == 0) {
        Serial.println("{\"type\":\"ERROR\",\"event\":\"AUDIO_SERVER_URL_EMPTY\"}");
        return false;
    }

    String base = serverBaseUrl;
    if (!base.endsWith("/")) {
        base += "/";
    }

    String url = base + audioId + ".mp3";

    if (audio.isRunning()) {
        audio.stopSong();
        delay(60);
    }

    bool ok = audio.connecttohost(url.c_str());

    if (ok) {
        Serial.printf("{\"type\":\"SYSTEM\",\"event\":\"AUDIO_PLAY_WIFI\",\"url\":\"%s\"}\n", url.c_str());
    } else {
        Serial.printf("{\"type\":\"ERROR\",\"event\":\"AUDIO_PLAY_WIFI_FAILED\",\"url\":\"%s\"}\n", url.c_str());
    }

    return ok;
}

bool AudioService::playInternal(const String& audioId) {
    if (playFromSD(audioId)) {
        return true;
    }

    if (playFromWiFi(audioId)) {
        return true;
    }

    Serial.printf("{\"type\":\"ERROR\",\"event\":\"AUDIO_PLAY_ALL_FAILED\",\"audioId\":\"%s\"}\n", audioId.c_str());
    return false;
}

void AudioService::play(String audioId, bool loop) {
    audioId.trim();

    if (audioId.isEmpty()) {
        Serial.println("{\"type\":\"ERROR\",\"event\":\"AUDIO_ID_EMPTY\"}");
        return;
    }

    isLooping = loop;
    currentLoopFile = audioId;
    isSequencePlaying = false;
    audioQueue.clear();

    audio.setVolume(currentVol);
    playInternal(audioId);
}

void AudioService::playExplosion() {
    stop();
    setVolumePercentage(90);
    play("5_0_1_explosion", false);
}

void AudioService::playSequence(const std::vector<String>& ids) {
    stop();

    if (ids.empty()) {
        return;
    }

    audioQueue = ids;
    isSequencePlaying = true;

    // fuerte, pero no máximo para evitar recorte
    setVolumePercentage(85);

    String first = audioQueue.front();
    audioQueue.erase(audioQueue.begin());

    playInternal(first);
}

void AudioService::stop() {
    if (audio.isRunning()) {
        audio.stopSong();
        delay(60);
    }

    isSequencePlaying = false;
    isIncreasingVol = false;
    isLooping = false;
    currentLoopFile = "";
    audioQueue.clear();
}

void AudioService::setVolumePercentage(int percentage) {
    percentage = constrain(percentage, 0, 100);
    currentVol = map(percentage, 0, 100, 0, 21);
    audio.setVolume(currentVol);
}

void AudioService::startIncreasingVolume() {
    stop();

    // Arranque moderado para que el tic tac no salga tronado
    setVolumePercentage(45);

    isIncreasingVol = true;
    lastVolUpdate = millis();
    play("5_0_0_tic_tac", true);
}

void AudioService::loop() {
    audio.loop();

    if (isIncreasingVol && audio.isRunning()) {
        if (millis() - lastVolUpdate >= 500) {
            if (currentVol < 20) {
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
    if (isLooping && !currentLoopFile.isEmpty()) {
        playInternal(currentLoopFile);
        return;
    }

    if (isSequencePlaying && !audioQueue.empty()) {
        String next = audioQueue.front();
        audioQueue.erase(audioQueue.begin());
        playInternal(next);
        return;
    }

    isSequencePlaying = false;
}

AudioService audioService;

void audio_eof_mp3(const char *info) {
    (void)info;
    audioService.handleAudioEOF();
}