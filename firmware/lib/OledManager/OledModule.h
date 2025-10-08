#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OledModule {
public:
    OledModule(uint8_t address, int sdaPin = 21, int sclPin = 22)
        : _address(address), _sdaPin(sdaPin), _sclPin(sclPin), _display(nullptr) {}

    bool begin() {
        Wire.begin(_sdaPin, _sclPin);
        _display = new Adafruit_SSD1306(128, 64, &Wire, -1);
        if (!_display->begin(SSD1306_SWITCHCAPVCC, _address)) {
            Serial.printf("OLED at 0x%02X failed\n", _address);
            delete _display;
            _display = nullptr;
            return false;
        }
        _display->clearDisplay();
        _display->display();
        Serial.printf("OLED initialized at 0x%02X\n", _address);
        return true;
    }

    void showText(const String &text, uint8_t textSize = 1, uint8_t x = 0, uint8_t y = 0) {
        if (!_display) return;
        _display->clearDisplay();
        _display->setTextSize(textSize);
        _display->setTextColor(SSD1306_WHITE);
        _display->setCursor(x, y);
        _display->print(text);
        _display->display();
    }

    void clear() {
        if (!_display) return;
        _display->clearDisplay();
        _display->display();
    }

    Adafruit_SSD1306* display() { return _display; }

private:
    uint8_t _address;
    int _sdaPin;
    int _sclPin;
    Adafruit_SSD1306* _display;
};
