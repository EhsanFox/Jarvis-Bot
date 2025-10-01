#include "WiFiManager.h"

WiFiManager::WiFiManager(ConfigManager &cfg) : _config(cfg) {}

void WiFiManager::begin() {
    // no heavy work here; WiFi.begin() will be done in connectFromConfig
    WiFi.disconnect(true); // clear previous connection
    delay(100);
}

std::vector<String> WiFiManager::scanSSIDs() {
    std::vector<String> list;
    Serial.println("🔎 Scanning Wi-Fi networks...");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("⚠️ No networks found");
        return list;
    }
    for (int i = 0; i < n; ++i) {
        String line = String(WiFi.SSID(i)) + " (" + String(WiFi.RSSI(i)) + " dB)";
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) line += " [open]";
        list.push_back(line);
    }
    WiFi.scanDelete(); // cleanup
    return list;
}

bool WiFiManager::connectFromConfig(unsigned long timeoutMs) {
    WiFiConfig w = _config.getWiFiConfig();
    if (w.ssid.length() == 0) {
        Serial.println("⚠️ No SSID in config.json");
        return false;
    }

    Serial.printf("📡 Attempting Wi-Fi connect to '%s' ...\n", w.ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(w.ssid.c_str(), w.password.c_str());

    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("✅ Connected. IP: ");
            Serial.println(WiFi.localIP().toString());
            return true;
        }
        delay(200);
    }

    Serial.println("❌ Wi-Fi connect timed out");
    WiFi.disconnect(true);
    return false;
}

bool WiFiManager::startAP(const String &apSSID, const String &apPass) {
    Serial.println("⚠️ Starting Access Point (hotspot)...");
    WiFi.mode(WIFI_AP);
    bool ok;
    if (apPass.length() >= 8) {
        ok = WiFi.softAP(apSSID.c_str(), apPass.c_str());
    } else {
        ok = WiFi.softAP(apSSID.c_str()); // open AP
    }

    if (!ok) {
        Serial.println("❌ Failed to start AP");
        return false;
    }

    IPAddress apIp = WiFi.softAPIP();
    Serial.print("🔌 AP started. SSID: ");
    Serial.print(apSSID);
    if (apPass.length() >= 8) {
        Serial.print(" (secured)");
    } else {
        Serial.print(" (open)");
    }
    Serial.print("  IP: ");
    Serial.println(apIp.toString());

    // Call the callback so WebServerManager can mount files / start captive portal
    if (_apStartedCallback) {
        Serial.println("▶ Calling AP started callback");
        _apStartedCallback();
    }
    return true;
}

void WiFiManager::setAPStartedCallback(std::function<void()> cb) {
    _apStartedCallback = cb;
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::ipAddress() {
    if (isConnected()) return WiFi.localIP().toString();
    return WiFi.softAPIP().toString();
}
