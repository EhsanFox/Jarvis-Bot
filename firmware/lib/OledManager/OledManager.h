#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vector>

enum EyeExpression {
    NEUTRAL,
    HAPPY,
    SAD,
    ANGRY,
    SURPRISED,
    BLINK
};

struct OledDisplay {
    Adafruit_SSD1306* display;
    uint8_t address;
    bool isEye;
    float blinkLevel = 0.0f;
    int pupilX = 0;
    int pupilY = 0;
    bool hasEyebrow = false;
    EyeExpression expression = NEUTRAL;
};

class OledManager {
public:
    OledManager(uint8_t width, uint8_t height);
    void addDisplay(uint8_t address, bool isEye);
    void begin();
    void showText(uint8_t index, const char* text);
    void showEye(uint8_t index);
    void updateEyes();

private:
    std::vector<OledDisplay> displays;
    uint8_t w, h;
    unsigned long lastBlink = 0;
    bool blinking = false;

    void drawPixelEye(OledDisplay& d);
    void drawCozmoEye(OledDisplay& d);
};
