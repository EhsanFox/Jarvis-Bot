#include "WiFiManager.h"

WiFiManager::WiFiManager(const String& ssid, const String& password,
                         const String& apName, const String& apPassword)
    : _ssid(ssid), _password(password), _apName(apName), _apPassword(apPassword) {}

void WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
}

bool WiFiManager::connectSTA(unsigned long timeout) {
    if (_ssid.length() == 0) return false;

    WiFi.begin(_ssid.c_str(), _password.c_str());
    Serial.print("Connecting to Wi-Fi");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::startAP() {
    WiFi.mode(WIFI_AP);
    bool res;
    if (_apPassword.length() == 0) {
        res = WiFi.softAP(_apName.c_str());
    } else {
        res = WiFi.softAP(_apName.c_str(), _apPassword.c_str());
    }

    if (res) {
        Serial.print("AP started! IP: ");
        Serial.println(WiFi.softAPIP());
        if (_apStartedCallback) _apStartedCallback();
    } else {
        Serial.println("❌ Failed to start AP!");
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::ipAddress() const {
    if (WiFi.getMode() == WIFI_STA) {
        return WiFi.localIP().toString();
    } else {
        return WiFi.softAPIP().toString();
    }
}
