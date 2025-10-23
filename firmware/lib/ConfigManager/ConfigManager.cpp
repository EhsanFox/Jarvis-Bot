#include "ConfigManager.h"

ConfigManager::ConfigManager(const char* filePath) : _filePath(filePath), _doc(1024) {
    if (!LittleFS.begin(true)) {
        Serial.println("❌ Failed to mount LittleFS");
    }
    load();
}

// Load JSON file into _doc
bool ConfigManager::load() {
    if (!LittleFS.exists(_filePath)) {
        Serial.println("⚠️ Config file does not exist, creating new...");
        _doc.clear();
        save();
        return true;
    }

    File file = LittleFS.open(_filePath, "r");
    if (!file) {
        Serial.println("❌ Failed to open config file for reading");
        return false;
    }

    DeserializationError err = deserializeJson(_doc, file);
    file.close();

    if (err) {
        Serial.print("❌ Failed to parse config.json: ");
        Serial.println(err.c_str());
        _doc.clear();
        return false;
    }

    return true;
}

// Save _doc to file
bool ConfigManager::save() {
    File file = LittleFS.open(_filePath, "w");
    if (!file) {
        Serial.println("❌ Failed to open config file for writing");
        return false;
    }

    if (serializeJson(_doc, file) == 0) {
        Serial.println("❌ Failed to write JSON to file");
        file.close();
        return false;
    }

    file.close();
    return true;
}

// Public get: returns JsonVariant, can be string, object, number, etc.
JsonVariant ConfigManager::get(const String& key) {
    // Split key by '.'
    int start = 0;
    JsonVariant current = _doc;

    while (true) {
        int dotIndex = key.indexOf('.', start);
        String part = (dotIndex == -1) ? key.substring(start) : key.substring(start, dotIndex);

        if (!current.containsKey(part)) {
            return JsonVariant(); // not found
        }

        current = current[part];
        if (dotIndex == -1) break; // last segment
        start = dotIndex + 1;
    }

    return current;
}

// Public set: assign value to key, value can be primitive or JSON object
bool ConfigManager::set(const String& key, const JsonVariant& value) {
    _doc[key] = value;
    return save();
}
bool ConfigManager::set(const String& key, const String& value) {
    _doc[key] = value;
    return save();
}
bool ConfigManager::set(const String& key, const bool& value) {
    _doc[key] = value;
    return save();
}
bool ConfigManager::set(const String& key, const int& value) {
    _doc[key] = value;
    return save();
}