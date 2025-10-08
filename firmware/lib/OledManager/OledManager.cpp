#include <Arduino.h>
#include "OledManager.h"
#include <Wire.h>
#include <algorithm>

OledManager::OledManager(uint8_t width, uint8_t height) : w(width), h(height) {}

void OledManager::addDisplay(uint8_t address, bool isEye) {
    auto* disp = new Adafruit_SSD1306(w, h, &Wire, -1);
    OledDisplay od;
    od.display = disp;
    od.address = address;
    od.isEye = isEye;
    displays.push_back(od);
}

void OledManager::begin() {
    for (auto& d : displays) {
        if (!d.display->begin(SSD1306_SWITCHCAPVCC, d.address)) {
            Serial.printf("OLED at 0x%X failed\n", d.address);
            continue;
        }
        d.display->clearDisplay();
        d.display->display();
    }
}

void OledManager::showText(uint8_t index, const char* text) {
    if (index >= displays.size()) return;
    auto& d = displays[index];
    d.display->clearDisplay();
    d.display->setTextSize(1);
    d.display->setTextColor(SSD1306_WHITE);
    d.display->setCursor(0, 0);
    d.display->println(text);
    d.display->display();
}

void OledManager::drawCozmoEye(OledDisplay& d) {
    Adafruit_SSD1306* disp = d.display;
    disp->clearDisplay();

    int eyeW = w;
    int eyeH = h;
    int cx = eyeW / 2 + d.pupilX;
    int cy = eyeH / 2 + d.pupilY;

    // --- Eye whites (no border) ---
    disp->fillCircle(cx, cy, eyeW / 2, SSD1306_WHITE);
    disp->fillCircle(cx, cy, (eyeW / 2) - 2, SSD1306_BLACK);

    // --- Pupil (rounded) ---
    int pupilRadius = 3;
    disp->fillCircle(cx, cy, pupilRadius, SSD1306_WHITE);

    // --- Optional eyebrow ---
    if (d.hasEyebrow) {
        int browY = cy - eyeH / 4;
        if (d.expression == ANGRY)
            disp->drawLine(cx - 4, browY + 1, cx + 4, browY - 1, SSD1306_WHITE);
        else if (d.expression == SAD)
            disp->drawLine(cx - 4, browY - 1, cx + 4, browY + 1, SSD1306_WHITE);
        else
            disp->drawLine(cx - 4, browY, cx + 4, browY, SSD1306_WHITE);
    }

    // --- Blinking overlay ---
    if (blinking) {
        int blinkHeight = eyeH * d.blinkLevel;
        disp->fillRect(cx - eyeW / 2, cy - blinkHeight / 2, eyeW, blinkHeight, SSD1306_BLACK);
    }

    disp->display();
}


void OledManager::drawPixelEye(OledDisplay& d) {
    Adafruit_SSD1306* disp = d.display;
    disp->clearDisplay();

    int eyeW = w;
    int eyeH = h;
    int cx = eyeW / 2;
    int cy = eyeH / 2;

    // --- Draw rounded eye background (Cozmo-style) ---
    int cornerRadius = 4; // bigger radius for rounder look
    disp->fillRoundRect(0, 0, eyeW, eyeH, cornerRadius, SSD1306_BLACK);
    disp->drawRoundRect(0, 0, eyeW, eyeH, cornerRadius, SSD1306_WHITE);

    // --- Handle blinking ---
    if (blinking) {
        int blinkHeight = eyeH * d.blinkLevel;
        disp->fillRect(0, cy - blinkHeight / 2, eyeW, blinkHeight, SSD1306_WHITE);
        disp->display();
        return;
    }

    // --- Draw pupil ---
    int pupilSize = eyeW / 4; // Cozmo-like proportion
    int pupilX = cx + d.pupilX - pupilSize / 2;
    int pupilY = cy + d.pupilY - pupilSize / 2;
    disp->fillRect(pupilX, pupilY, pupilSize, pupilSize, SSD1306_WHITE);

    // --- Optional eyebrow / expression ---
    if (d.hasEyebrow) {
        switch (d.expression) {
            case ANGRY:
                disp->drawLine(2, 4, eyeW / 2, 0, SSD1306_WHITE); // slanted down
                disp->drawLine(eyeW / 2, 0, eyeW - 2, 4, SSD1306_WHITE);
                break;
            case SAD:
                disp->drawLine(2, 0, eyeW / 2, 4, SSD1306_WHITE); // slanted up
                disp->drawLine(eyeW / 2, 4, eyeW - 2, 0, SSD1306_WHITE);
                break;
            case HAPPY:
                disp->drawLine(2, 2, eyeW - 2, 2, SSD1306_WHITE); // flat
                break;
            default:
                disp->drawLine(2, 2, eyeW - 2, 2, SSD1306_WHITE); // NEUTRAL
                break;
        }
    }

    disp->display();
}


void OledManager::showEye(uint8_t index) {
    if (index >= displays.size()) return;
    auto& d = displays[index];
    drawCozmoEye(d);
}

void OledManager::updateEyes() {
    unsigned long now = millis();

    if (!blinking && now - lastBlink > random(3000, 6000)) {
        blinking = true;
        lastBlink = now;
    }

    for (auto& d : displays) {
        if (!d.isEye) continue;

        if (blinking) {
            d.blinkLevel = std::min(1.0f, d.blinkLevel + 0.2f);
            if (d.blinkLevel >= 1.0f) {
                delay(80);
                blinking = false;
                d.blinkLevel = 0.0f;
                lastBlink = now;
            }
        } else {
            if (random(0, 20) == 0) {
                d.pupilX = random(-2, 3);
                d.pupilY = random(-2, 3);
            }
        }

        drawPixelEye(d);
    }
}
