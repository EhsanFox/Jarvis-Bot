#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <functional>
#include <vector>

struct WiFiNetwork {
    String ssid;
    int32_t rssi;
    bool secure;
    bool hidden;

    JsonObject toJson(JsonArray& arr) const {
        JsonObject obj = arr.createNestedObject();
        obj["ssid"] = ssid;
        obj["rssi"] = rssi;
        obj["secure"] = secure;
        obj["hidden"] = hidden;
        return obj;
    }
};

class WiFiManager {
public:
    WiFiManager(const String& ssid = "", const String& password = "",
                const String& apName = "ESP32-AP", const String& apPassword = "");

    void begin();
    bool connectSTA(unsigned long timeout = 10000);
    void startAP();                               
    bool isConnected() const;
    String ipAddress() const;
    void startScan();
    bool isScanComplete() const;
    DynamicJsonDocument getScanResults();
    void setAPStartedCallback(std::function<void()> cb) { _apStartedCallback = cb; }
    void stopAP();
    String tryConnect(const String& ssid, const String& password, unsigned long timeout = 10000);
    bool connectTo(const String& ssid, const String& password = "", unsigned long timeout = 10000);
    void disconnect();

private:
    String _ssid;
    String _password;
    String _apName;
    String _apPassword;

    std::function<void()> _apStartedCallback;
};
