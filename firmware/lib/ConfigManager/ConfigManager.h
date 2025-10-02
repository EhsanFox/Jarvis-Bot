#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class ConfigManager {
public:
    ConfigManager(const char* filePath = "/config.json");
    
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }
    // Public API
    JsonVariant get(const String& key);             // get value for a key
    bool set(const String& key, const JsonVariant& value);  // set value for a key

private:
    const char* _filePath;
    DynamicJsonDocument _doc;

    bool load();   // load JSON from file
    bool save();   // save JSON to file
};
