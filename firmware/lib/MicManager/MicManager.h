#pragma once
#include <Arduino.h>
#include "driver/i2s.h"

class MicManager {
public:
    MicManager(
        int pinBCLK,
        int pinLRCLK,
        int pinDOUT,
        uint32_t sampleRate = 16000,
        i2s_port_t i2sPort = I2S_NUM_0
    );

    bool begin();
    bool readSamples(int32_t* buffer, size_t sampleCount, size_t& samplesRead);
    float calculateRMS(int32_t* buffer, size_t samples);
    float calculateDB(float rms);

private:
    int _pinBCLK;
    int _pinLRCLK;
    int _pinDOUT;

    uint32_t _sampleRate;
    i2s_port_t _i2sPort;
};
