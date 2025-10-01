#include <Arduino.h>
#include "ConfigManager.h"
#include "WiFiManager.h"
// #include "WebServerManager.h" // your future webserver module

ConfigManager config;
WiFiManager *wifiManager = nullptr; // pointer so we can set callback after creating webserver
// WebServerManager webServer(config); // example

void setup() {
    // Disable brownout detector (temporary debug only)
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    delay(1000);
    Serial.println("Booting...");

    config.begin(); // mount LittleFS and load config

    // create wifiManager
    wifiManager = new WiFiManager(config);
    wifiManager->begin();

    // optional: set AP-start callback to start your webserver when AP is active
    // wifiManager->setAPStartedCallback([&]() {
    //     webServer.beginAP(); // start AsyncWebServer, mount LittleFS files, captive portal, etc.
    // });

    // Try to connect from config for 10 seconds
    if (!wifiManager->connectFromConfig(10000)) {
        // connect failed -> start AP for phone to connect and configure
        wifiManager->startAP("AI-Bot-Setup", ""); // open AP; change pass if you want
    } else {
        Serial.print("Connected. IP: ");
        Serial.println(wifiManager->ipAddress());
        // optionally start webserver in station mode:
        // webServer.beginSTA();
    }
}

void loop() {
    // optional: print status every few seconds
    static unsigned long last = 0;
    if (millis() - last > 3000) {
        last = millis();
        Serial.print("WiFi connected: ");
        Serial.println(wifiManager->isConnected() ? "YES" : "NO");
        Serial.print("IP: "); Serial.println(wifiManager->ipAddress());
    }
}
