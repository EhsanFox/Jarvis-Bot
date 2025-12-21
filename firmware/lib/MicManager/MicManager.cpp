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
        .communication_format = I2S_COMM_FORMAT_STAND_I2S, // Updated from deprecated I2S_COMM_FORMAT_I2S
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

int32_t MicManager::convertSample24to16(int32_t sample) {
    // Extract 24-bit audio (shift right by 8 to remove padding)
    int32_t sample24 = sample >> 8;
    
    // Convert to 16-bit
    return sample24 / 256;
}

bool MicManager::writeWavHeader(File& file, uint32_t dataSize, uint32_t sampleRate, uint16_t bitsPerSample) {
    // Calculate file sizes
    uint32_t byteRate = sampleRate * 1 * bitsPerSample / 8; // 1 channel
    uint16_t blockAlign = 1 * bitsPerSample / 8;
    uint32_t chunkSize = 36 + dataSize;
    uint32_t subchunk2Size = dataSize;
    
    // RIFF header
    file.write((uint8_t*)"RIFF", 4);
    file.write((uint8_t*)&chunkSize, 4);
    file.write((uint8_t*)"WAVE", 4);
    
    // fmt subchunk
    file.write((uint8_t*)"fmt ", 4);
    uint32_t subchunk1Size = 16;
    file.write((uint8_t*)&subchunk1Size, 4);
    uint16_t audioFormat = 1; // PCM
    file.write((uint8_t*)&audioFormat, 2);
    uint16_t numChannels = 1; // Mono
    file.write((uint8_t*)&numChannels, 2);
    file.write((uint8_t*)&sampleRate, 4);
    file.write((uint8_t*)&byteRate, 4);
    file.write((uint8_t*)&blockAlign, 2);
    file.write((uint8_t*)&bitsPerSample, 2);
    
    // data subchunk
    file.write((uint8_t*)"data", 4);
    file.write((uint8_t*)&subchunk2Size, 4);
    
    return true;
}

void MicManager::updateWavHeader() {
    if (_recordFile) {
        // Save current position
        size_t currentPos = _recordFile.position();
        
        // Go to beginning and update header
        _recordFile.seek(0);
        writeWavHeader(_recordFile, _recordedBytes, _sampleRate, 16);
        
        // Restore position
        _recordFile.seek(currentPos);
    }
}

bool MicManager::startRecording(const String& filename) {
    // Check if already recording
    if (_isRecording) {
        return false;
    }
    
    // Create file
    _recordFile = LittleFS.open(filename, "w");
    if (!_recordFile) {
        return false;
    }
    
    _currentFilename = filename;
    
    // Reserve space for WAV header
    uint32_t headerSize = 44; // Standard WAV header size
    for (uint32_t i = 0; i < headerSize; i++) {
        _recordFile.write((uint8_t)0);
    }
    
    // Reset recording stats
    _recordStartTime = millis();
    _recordedDuration = 0;
    _recordedBytes = 0;
    _isRecording = true;
    
    // Create recording task
    xTaskCreatePinnedToCore(
        recordingTask,      // Task function
        "RecordingTask",    // Task name
        4096,               // Stack size
        this,               // Parameters
        1,                  // Priority
        &_recordingTaskHandle, // Task handle
        1                   // Core (use core 1, leaving core 0 for WiFi/BT)
    );
    
    return true;
}

bool MicManager::stopRecording() {
    if (!_isRecording) {
        return false;
    }
    
    // Signal recording to stop
    _isRecording = false;
    
    // Wait for task to finish
    if (_recordingTaskHandle) {
        // Wait for task to complete (max 2 seconds)
        uint32_t timeout = millis() + 2000;
        while (_recordingTaskHandle != nullptr && millis() < timeout) {
            delay(10);
        }
        
        // Force delete if still running
        if (_recordingTaskHandle) {
            vTaskDelete(_recordingTaskHandle);
            _recordingTaskHandle = nullptr;
        }
    }
    
    // Update WAV header with final size
    updateWavHeader();
    
    // Close file
    if (_recordFile) {
        _recordFile.close();
    }
    
    // Calculate final duration
    _recordedDuration = millis() - _recordStartTime;
    
    return true;
}

void MicManager::recordingTask(void* parameter) {
    MicManager* instance = static_cast<MicManager*>(parameter);
    instance->recordingLoop();
    
    // Clean up task handle
    instance->_recordingTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

void MicManager::recordingLoop() {
    // FIX 1: Use static buffer to avoid heap fragmentation
    static const size_t BUFFER_SIZE = 512;
    static int32_t readBuffer[BUFFER_SIZE];
    
    uint32_t lastCallbackTime = 0;
    uint32_t lastStatsTime = 0;
    uint32_t totalSamplesWritten = 0;
    uint32_t bytesProcessedThisCycle = 0;
    
    // FIX 2: Adaptive yield mechanism
    const uint32_t MAX_BYTES_PER_CYCLE = 4096; // Process max 4KB before yielding
    const TickType_t YIELD_DELAY = pdMS_TO_TICKS(5); // Yield for 5ms
    
    while (_isRecording && _recordFile) {
        size_t samplesRead = 0;
        
        // FIX 3: Yield CPU periodically to prevent hogging
        if (bytesProcessedThisCycle >= MAX_BYTES_PER_CYCLE) {
            vTaskDelay(YIELD_DELAY);
            bytesProcessedThisCycle = 0;
            continue;
        }
        
        // Read samples with timeout
        if (!readSamples(readBuffer, BUFFER_SIZE, samplesRead) || samplesRead == 0) {
            // FIX 4: Small delay when no data instead of tight loop
            vTaskDelay(1);
            continue;
        }
        
        // Process and write samples
        for (size_t i = 0; i < samplesRead; i++) {
            // Convert 24-bit to 16-bit
            int16_t sample16 = (int16_t)convertSample24to16(readBuffer[i]);
            
            // Write to file
            if (_recordFile.write((uint8_t*)&sample16, sizeof(int16_t)) != sizeof(int16_t)) {
                // Write failed
                _isRecording = false;
                break;
            }
            _recordedBytes += sizeof(int16_t);
            totalSamplesWritten++;
            bytesProcessedThisCycle += sizeof(int16_t);
        }
        
        // Update duration
        _recordedDuration = millis() - _recordStartTime;
        
        // Call callback every 500ms (non-blocking check)
        if (_statusCallback && (millis() - lastCallbackTime > 500)) {
            float rms = calculateRMS(readBuffer, min(samplesRead, BUFFER_SIZE));
            float db = calculateDB(rms);
            _statusCallback(_recordedDuration, _recordedBytes, db);
            lastCallbackTime = millis();
        }
        
        // FIX 5: Optional stats - only if serial buffer isn't full
        if (millis() - lastStatsTime > 5000 && Serial.availableForWrite() > 100) {
            Serial.printf("[MIC] Recording: %ds, %dKB, Heap: %d\n", 
                         _recordedDuration/1000, _recordedBytes/1024, ESP.getFreeHeap());
            lastStatsTime = millis();
        }
        
        // FIX 6: Strategic yield based on workload
        // More samples = less yield needed, but always yield a little
        if (samplesRead > 0) {
            // Yield 1-3ms based on how much work we did
            TickType_t adaptiveDelay = pdMS_TO_TICKS(1 + (samplesRead / 256));
            vTaskDelay(adaptiveDelay);
        }
    }
    
    // No delete needed since buffer is static
    
    // Clean up if recording stopped
    if (!_isRecording && _recordFile) {
        updateWavHeader();
        _recordFile.flush();
        _recordFile.close();
    }
    
    // FIX 7: Clear task handle before exiting
    _recordingTaskHandle = nullptr;
}