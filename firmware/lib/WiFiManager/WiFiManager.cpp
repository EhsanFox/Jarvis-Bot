#include "WiFiManager.h"

WiFiManager::WiFiManager(const String& ssid, const String& password,
                         const String& apName, const String& apPassword)
    : _ssid(ssid), _password(password), _apName(apName), _apPassword(apPassword) {}

void WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
}

void WiFiManager::startScan() {
    WiFi.scanDelete();
    WiFi.scanNetworks(true); // async scan
}

bool WiFiManager::isScanComplete() const {
    return WiFi.scanComplete() >= 0;
}

DynamicJsonDocument WiFiManager::getScanResults() {
    int n = WiFi.scanComplete();
    DynamicJsonDocument doc(2048);
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < n; ++i) {
        JsonObject obj = arr.createNestedObject();
        obj["ssid"] = WiFi.SSID(i);
        obj["rssi"] = WiFi.RSSI(i);
        obj["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        obj["hidden"] = false;
    }
    // WiFi.scanDelete(); // clean up old results
    startScan();
    return doc;
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

/*
bool WiFiManager::connectTo(const String& ssid, const String& password = "", unsigned long timeout = 10000) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to Wi-Fi");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}
*/

void WiFiManager::disconnect() {
     if (WiFi.isConnected()) {
        Serial.println("🔌 Disconnecting from Wi-Fi...");
        WiFi.disconnect(true);  // true = erase credentials
    }

    // Switch to AP mode by calling existing method
    Serial.println("📡 Switching to Access Point mode...");
    startAP();
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

        startScan();
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
