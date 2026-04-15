#include "MicService.h"
#include "AudioService.h"

MicService::MicService()
    : _isStreaming(false), _isReady(false) {}

void MicService::setup() {
    i2s_config_t i2s_config = {};
    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
    i2s_config.sample_rate = 16000;
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    i2s_config.dma_buf_count = 8;
    i2s_config.dma_buf_len = 256;
    i2s_config.use_apll = false;
    i2s_config.tx_desc_auto_clear = false;
    i2s_config.fixed_mclk = 0;

    i2s_pin_config_t pin_config = {};
    pin_config.bck_io_num = I2S_MIC_SCK;
    pin_config.ws_io_num = I2S_MIC_WS;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = I2S_MIC_SD;

    i2s_driver_uninstall(I2S_MIC_PORT);

    esp_err_t err = i2s_driver_install(I2S_MIC_PORT, &i2s_config, 0, nullptr);
    if (err != ESP_OK) {
        Serial.printf("{\"type\":\"ERROR\",\"event\":\"MIC_DRIVER_INSTALL_FAILED\",\"code\":%d}\n", err);
        _isReady = false;
        return;
    }

    err = i2s_set_pin(I2S_MIC_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("{\"type\":\"ERROR\",\"event\":\"MIC_SET_PIN_FAILED\",\"code\":%d}\n", err);
        i2s_driver_uninstall(I2S_MIC_PORT);
        _isReady = false;
        return;
    }

    err = i2s_set_clk(I2S_MIC_PORT, 16000, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_MONO);
    if (err != ESP_OK) {
        Serial.printf("{\"type\":\"ERROR\",\"event\":\"MIC_SET_CLK_FAILED\",\"code\":%d}\n", err);
        i2s_driver_uninstall(I2S_MIC_PORT);
        _isReady = false;
        return;
    }

    i2s_zero_dma_buffer(I2S_MIC_PORT);
    i2s_stop(I2S_MIC_PORT);

    _isReady = true;
    Serial.println("{\"type\":\"SYSTEM\",\"event\":\"MIC_READY\"}");
}

void MicService::startStreaming() {
    if (!_isReady || _isStreaming) return;

    audioService.stop();
    delay(40);

    i2s_zero_dma_buffer(I2S_MIC_PORT);
    i2s_start(I2S_MIC_PORT);

    _isStreaming = true;
    Serial.println("{\"type\":\"MIC_START\"}");
}

void MicService::stopStreaming() {
    if (!_isStreaming) return;

    _isStreaming = false;
    i2s_stop(I2S_MIC_PORT);

    Serial.println("{\"type\":\"MIC_STOP\"}");
}

void MicService::update() {
    if (!_isStreaming || !_isReady) return;

    size_t bytesRead = 0;
    esp_err_t result = i2s_read(
        I2S_MIC_PORT,
        rawBuffer,
        sizeof(rawBuffer),
        &bytesRead,
        pdMS_TO_TICKS(20)
    );

    if (result != ESP_OK || bytesRead == 0) return;

    const size_t samplesRead = bytesRead / sizeof(int32_t);
    if (samplesRead == 0) return;

    int32_t mean = 0;
    int32_t peak = 0;

    for (size_t i = 0; i < samplesRead; i++) {
        int32_t sample = rawBuffer[i] >> 15;
        pcmBuffer[i] = (int16_t)sample;
        mean += pcmBuffer[i];

        int32_t absSample = sample >= 0 ? sample : -sample;
        if (absSample > peak) peak = absSample;
    }

    mean /= (int32_t)samplesRead;

    for (size_t i = 0; i < samplesRead; i++) {
        int32_t sample = (int32_t)pcmBuffer[i] - mean;
        sample *= 2;

        if (sample > -100 && sample < 100) {
            sample = 0;
        }

        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;

        pcmBuffer[i] = (int16_t)sample;
    }

    if (peak < 40) return;

    const size_t outBytes = samplesRead * sizeof(int16_t);
    Serial.write((const uint8_t*)pcmBuffer, outBytes);
}

MicService micService;