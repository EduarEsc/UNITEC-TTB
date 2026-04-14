#include "MicService.h"

extern WebSocketsClient webSocket;

MicService::MicService() : _isStreaming(false) {}

void MicService::setup()
{
    // Configuración para el micrófono I2S (INMP441 o similar)
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // RX para recibir audio
        .sample_rate = 16000,                                // 16kHz es ideal para reconocimiento de voz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // El micro suele ser canal único
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false};

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK_SCK, // Pin compartido 05
        .ws_io_num = I2S_LRC_WS,    // Pin compartido 04
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_MIC_SD // Pin exclusivo 01
    };

    // Instalamos el driver en el puerto 0
    esp_err_t err = i2s_driver_install(I2S_MIC_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        Serial.printf("❌ Error instalando I2S Mic: %d\n", err);
        return;
    }

    err = i2s_set_pin(I2S_MIC_PORT, &pin_config);
    if (err != ESP_OK)
    {
        Serial.printf("❌ Error seteando pines Mic: %d\n", err);
    }

    Serial.println("✅ MicService (I2S) configurado.");
}

void MicService::startStreaming()
{
    _isStreaming = true;
    Serial.println("🎤 Micrófono abierto...");
}

void MicService::stopStreaming()
{
    _isStreaming = false;
    Serial.println("🛑 Micrófono cerrado.");
}

void MicService::update()
{
    if (!_isStreaming)
        return;

    size_t bytesRead = 0;
    // Leemos del buffer I2S
    esp_err_t result = i2s_read(I2S_MIC_PORT, &buffer, MIC_BUFFER_SIZE, &bytesRead, 10 / portTICK_PERIOD_MS);

    if (result == ESP_OK && bytesRead > 0)
    {
        // Enviamos el audio binario directamente al Backend de Python (FastAPI/WebSockets)
        // El backend debe estar preparado para recibir bytes raw de 16bit 16kHz
        webSocket.sendBIN(buffer, bytesRead);
    }
}

// Instancia global
MicService micService;