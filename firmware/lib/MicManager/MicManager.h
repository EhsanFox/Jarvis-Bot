#pragma once
#include <Arduino.h>
#include <LittleFS.h>
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
    
    // Recording control functions
    bool startRecording(const String& filename);
    bool stopRecording();
    
    // Recording status
    bool isRecording() const { return _isRecording; }
    uint32_t getRecordedDuration() const { return _recordedDuration; }
    size_t getRecordedBytes() const { return _recordedBytes; }
    String getCurrentFilename() const { return _currentFilename; }
    
    // Recording callback
    typedef std::function<void(uint32_t duration, size_t bytes, float currentDB)> RecordingStatusCallback;
    void setRecordingCallback(RecordingStatusCallback callback) { _statusCallback = callback; }

private:
    int _pinBCLK;
    int _pinLRCLK;
    int _pinDOUT;

    uint32_t _sampleRate;
    i2s_port_t _i2sPort;
    
    // Recording state
    bool _isRecording = false;
    File _recordFile;
    String _currentFilename;
    uint32_t _recordStartTime = 0;
    uint32_t _recordedDuration = 0;
    size_t _recordedBytes = 0;
    
    // Task handle for recording in background
    TaskHandle_t _recordingTaskHandle = nullptr;
    
    // Callback
    RecordingStatusCallback _statusCallback = nullptr;
    
    // Helper methods
    bool writeWavHeader(File& file, uint32_t dataSize, uint32_t sampleRate, uint16_t bitsPerSample = 16);
    int32_t convertSample24to16(int32_t sample);
    void updateWavHeader();
    
    // Static task function
    static void recordingTask(void* parameter);
    void recordingLoop();
};