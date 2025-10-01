#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

struct WiFiConfig {
    String ssid;
    String password;
};

class ConfigManager {
public:
    ConfigManager(const char* filePath = "/config.json");

    // Initialize filesystem and load config
    bool begin();

    // Load config from LittleFS
    bool loadConfig();

    // Save current config to LittleFS
    bool saveConfig();

    // Getters
    WiFiConfig getWiFiConfig() const;

    // Setters
    void setWiFiConfig(const String& ssid, const String& password);

private:
    const char* _filePath;
    WiFiConfig _wifiConfig;
};
