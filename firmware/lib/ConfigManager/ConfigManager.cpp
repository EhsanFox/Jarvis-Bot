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
/*
JsonVariant ConfigManager::getAll() {
    return _doc
}
*/

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
    // Split key by dots
    JsonVariant current = _doc;
    int start = 0;

    while (true) {
        int dotIndex = key.indexOf('.', start);
        String part = (dotIndex == -1) ? key.substring(start) : key.substring(start, dotIndex);

        if (part.isEmpty()) return false; // invalid key

        // If this is the last key part, assign the value
        if (dotIndex == -1) {
            current[part] = value;
            break;
        }

        // Ensure the sub-object exists
        if (!current[part].is<JsonObject>()) {
            current[part] = _doc.createNestedObject();
        }

        // Move deeper into the object
        current = current[part];
        start = dotIndex + 1;
    }

    return save();
}

bool ConfigManager::set(const String& key, const String& value) {
    // Split key by dots
    JsonVariant current = _doc;
    int start = 0;

    while (true) {
        int dotIndex = key.indexOf('.', start);
        String part = (dotIndex == -1) ? key.substring(start) : key.substring(start, dotIndex);

        if (part.isEmpty()) return false; // invalid key

        // If this is the last key part, assign the value
        if (dotIndex == -1) {
            current[part] = value;
            break;
        }

        // Ensure the sub-object exists
        if (!current[part].is<JsonObject>()) {
            current[part] = _doc.createNestedObject();
        }

        // Move deeper into the object
        current = current[part];
        start = dotIndex + 1;
    }

    return save();
}
bool ConfigManager::set(const String& key, const bool& value) {
    // Split key by dots
    JsonVariant current = _doc;
    int start = 0;

    while (true) {
        int dotIndex = key.indexOf('.', start);
        String part = (dotIndex == -1) ? key.substring(start) : key.substring(start, dotIndex);

        if (part.isEmpty()) return false; // invalid key

        // If this is the last key part, assign the value
        if (dotIndex == -1) {
            current[part] = value;
            break;
        }

        // Ensure the sub-object exists
        if (!current[part].is<JsonObject>()) {
            current[part] = _doc.createNestedObject();
        }

        // Move deeper into the object
        current = current[part];
        start = dotIndex + 1;
    }

    return save();
}
bool ConfigManager::set(const String& key, const int& value) {
    // Split key by dots
    JsonVariant current = _doc;
    int start = 0;

    while (true) {
        int dotIndex = key.indexOf('.', start);
        String part = (dotIndex == -1) ? key.substring(start) : key.substring(start, dotIndex);

        if (part.isEmpty()) return false; // invalid key

        // If this is the last key part, assign the value
        if (dotIndex == -1) {
            current[part] = value;
            break;
        }

        // Ensure the sub-object exists
        if (!current[part].is<JsonObject>()) {
            current[part] = _doc.createNestedObject();
        }

        // Move deeper into the object
        current = current[part];
        start = dotIndex + 1;
    }

    return save();
}