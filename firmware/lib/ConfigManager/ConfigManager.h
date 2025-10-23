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
    
    JsonVariant get(const String& key);
    bool set(const String& key, const JsonVariant& value); 
    bool set(const String& key, const String& value); 
    bool set(const String& key, const bool& value);
    bool set(const String& key, const int& value);

private:
    const char* _filePath;
    DynamicJsonDocument _doc;

    bool load();   // load JSON from file
    bool save();   // save JSON to file
};
