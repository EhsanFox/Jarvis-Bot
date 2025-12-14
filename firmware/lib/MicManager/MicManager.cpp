#include "MicManager.h"

MicManager::MicManager(int pinBCLK, int pinLRCLK, int pinDOUT,
                       uint32_t sampleRate, i2s_port_t i2sPort)
    : _pinBCLK(pinBCLK),
      _pinLRCLK(pinLRCLK),
      _pinDOUT(pinDOUT),
      _sampleRate(sampleRate),
      _i2sPort(i2sPort) {}

bool MicManager::begin() {
    i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = _sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 512,
        .use_apll = false
    };

    i2s_pin_config_t pins = {
        .bck_io_num = _pinBCLK,
        .ws_io_num = _pinLRCLK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = _pinDOUT
    };

    if (i2s_driver_install(_i2sPort, &config, 0, NULL) != ESP_OK)
        return false;

    if (i2s_set_pin(_i2sPort, &pins) != ESP_OK)
        return false;

    return true;
}

bool MicManager::readSamples(int32_t* buffer, size_t sampleCount, size_t& samplesRead) {
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(
        _i2sPort,
        (void*)buffer,
        sampleCount * sizeof(int32_t),
        &bytesRead,
        100
    );

    if (result != ESP_OK || bytesRead == 0) {
        samplesRead = 0;
        return false;
    }

    samplesRead = bytesRead / sizeof(int32_t);
    return true;
}

float MicManager::calculateRMS(int32_t* buffer, size_t samples) {
    double sum = 0;

    for (size_t i = 0; i < samples; i++) {
        int32_t v = buffer[i] >> 8;           // convert 32-bit left-aligned to 24-bit
        double f = (double)v / 8388608.0;     // normalize (2^23)
        sum += f * f;
    }

    return sqrt(sum / samples);
}

float MicManager::calculateDB(float rms) {
    return 20.0f * log10(rms + 1e-9); // avoid -inf
}
