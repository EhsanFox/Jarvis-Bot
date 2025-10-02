#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <string.h>

class SHA256 {
public:
    SHA256();
    void update(const uint8_t* data, size_t len);
    void finalize(uint8_t* hash, size_t hashLen);
    void resetHMAC(const uint8_t* key, size_t keyLen);
    void finalizeHMAC(const uint8_t* key, size_t keyLen, uint8_t* hash, size_t hashLen);

private:
    void transform(const uint8_t* chunk);
    uint32_t _state[8];
    uint64_t _bitCount;
    uint8_t _buffer[64];
    size_t _bufferLen;
};
