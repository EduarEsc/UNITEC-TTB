#include "AudioService.h"

AudioService::AudioService() : isSequencePlaying(false),
                               lastVolUpdate(0),
                               currentVol(6),
                               isIncreasingVol(false),
                               isLooping(false)
{
}

void AudioService::setup()
{
    // Inicializar LittleFS
    if (!LittleFS.begin(true))
    {
        Serial.println("❌ Error al montar LittleFS");
        return;
    }

    // Configuración de pines I2S del HardwareMap
    audio.setPinout(I2S_BCLK_SCK, I2S_LRC_WS, I2S_DOUT);
    setVolumePercentage(40);
    Serial.println("✅ Audio Service listo (LittleFS)");
}

void AudioService::setVolumePercentage(int percentage)
{
    currentVol = map(percentage, 0, 100, 0, 21);
    audio.setVolume(currentVol);
}

void AudioService::startIncreasingVolume()
{
    setVolumePercentage(35); // Inicia suave
    isIncreasingVol = true;
    lastVolUpdate = millis();
}

void AudioService::play(String audioId, bool loop)
{
    isLooping = loop;
    currentLoopFile = audioId;

    // Buscamos el archivo en la memoria Flash
    String path = "/" + audioId + ".mp3";
    Serial.println("🔊 Reproduciendo desde Flash: " + path);

    audio.connecttoFS(LittleFS, path.c_str());
}

void AudioService::playExplosion()
{
    stop();
    setVolumePercentage(100); // Máximo estruendo
    play("5_0_1_explosion", false);
}

void AudioService::playSequence(std::vector<String> ids)
{
    isLooping = false;
    audioQueue = ids;
    isSequencePlaying = true;
    if (!audioQueue.empty())
    {
        play(audioQueue[0]);
        audioQueue.erase(audioQueue.begin());
    }
}

void AudioService::stop()
{
    audio.stopSong();
    isSequencePlaying = false;
    isIncreasingVol = false;
    isLooping = false;
    audioQueue.clear();
}

void AudioService::loop()
{
    audio.loop();

    // Lógica de volumen ascendente para el Tic-Tac
    if (isIncreasingVol && audio.isRunning())
    {
        if (millis() - lastVolUpdate > 450)
        { // Sube cada medio segundo aprox
            if (currentVol < 21)
            {
                currentVol++;
                audio.setVolume(currentVol);
            }
            else
            {
                isIncreasingVol = false;
            }
            lastVolUpdate = millis();
        }
    }
}

// Instancia global
AudioService audioService;

// --- CALLBACKS DE LA LIBRERÍA ESP32-audioI2S ---
void audio_eof_mp3(const char *info)
{
    Serial.print("Fin de audio: ");
    Serial.println(info);

    if (audioService.isLooping)
    {
        audioService.play(audioService.currentLoopFile, true);
    }
    else if (audioService.getIsSequencePlaying() && !audioService.getAudioQueue().empty())
    {
        String next = audioService.getAudioQueue()[0];
        audioService.getAudioQueue().erase(audioService.getAudioQueue().begin());
        audioService.play(next);
    }
}