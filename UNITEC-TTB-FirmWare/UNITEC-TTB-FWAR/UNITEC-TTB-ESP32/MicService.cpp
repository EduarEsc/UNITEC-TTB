#include "MicService.h"
#include "AudioService.h" 

MicService::MicService() : _isStreaming(false) {}

void MicService::setup() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000, 
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Importante: L/R a GND
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,    // Más buffers evita cortes en el envío
        .dma_buf_len = 256,    
        .use_apll = false,
        .tx_desc_auto_clear = true
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num   = I2S_MIC_SCK, 
        .ws_io_num    = I2S_MIC_WS,  
        .data_out_num = I2S_PIN_NO_CHANGE, 
        .data_in_num  = I2S_MIC_SD   
    };

    // Instalamos en el puerto 0
    esp_err_t err = i2s_driver_install(I2S_MIC_PORT, &i2s_config, 0, NULL);

    if (err == ESP_OK) {
        i2s_set_pin(I2S_MIC_PORT, &pin_config);
        i2s_stop(I2S_MIC_PORT); // Lo dejamos en pausa hasta que se pida
        Serial.println("{\"type\":\"SYSTEM\", \"msg\":\"Mic_Config_OK\"}");
    }
}

void MicService::startStreaming() {
    if (!_isStreaming) {
        audioService.stop(); // Silencio total para escuchar al jugador
        _isStreaming = true;
        i2s_start(I2S_MIC_PORT); 
        i2s_zero_dma_buffer(I2S_MIC_PORT); 
        Serial.println("{\"type\":\"MIC_START\"}");
        Serial.flush();
    }
}

void MicService::stopStreaming() {
    if (_isStreaming) {
        _isStreaming = false;
        i2s_stop(I2S_MIC_PORT); 
        Serial.println("{\"type\":\"MIC_STOP\"}");
        Serial.flush();
    }
}

void MicService::update() {
    if (!_isStreaming) return;

    size_t bytesRead = 0;
    // Usamos el puerto 0 definido en el hardware map
    esp_err_t result = i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytesRead, 10);

    if (result == ESP_OK && bytesRead > 0) {
        if (bytesRead % 2 != 0) bytesRead--; 
        Serial.write((uint8_t*)buffer, bytesRead);
    }
}

MicService micService;