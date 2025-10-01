#include "ConfigManager.h"

ConfigManager::ConfigManager(const char* filePath)
    : _filePath(filePath)
{
}

bool ConfigManager::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("❌ Failed to mount LittleFS");
        return false;
    }

    // Try to load config, if fails, create default
    if (!loadConfig()) {
        Serial.println("⚠️ No config found, creating default.");
        _wifiConfig = {"", ""};
        saveConfig();
    }

    return true;
}

bool ConfigManager::loadConfig() {
    if (!LittleFS.exists(_filePath)) {
        Serial.println("⚠️ Config file does not exist.");
        return false;
    }

    File file = LittleFS.open(_filePath, "r");
    if (!file) {
        Serial.println("❌ Failed to open config file.");
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("❌ Failed to parse config.json: ");
        Serial.println(error.c_str());
        return false;
    }

    _wifiConfig.ssid = doc["ssid"] | "";
    _wifiConfig.password = doc["password"] | "";

    Serial.println("✅ Config loaded successfully.");
    Serial.print("SSID: "); Serial.println(_wifiConfig.ssid);

    return true;
}

bool ConfigManager::saveConfig() {
    File file = LittleFS.open(_filePath, "w");
    if (!file) {
        Serial.println("❌ Failed to open config file for writing.");
        return false;
    }

    DynamicJsonDocument doc(512);
    doc["ssid"] = _wifiConfig.ssid;
    doc["password"] = _wifiConfig.password;

    if (serializeJson(doc, file) == 0) {
        Serial.println("❌ Failed to write config to file.");
        file.close();
        return false;
    }

    file.close();
    Serial.println("✅ Config saved successfully.");
    return true;
}

WiFiConfig ConfigManager::getWiFiConfig() const {
    return _wifiConfig;
}

void ConfigManager::setWiFiConfig(const String& ssid, const String& password) {
    _wifiConfig.ssid = ssid;
    _wifiConfig.password = password;
}
