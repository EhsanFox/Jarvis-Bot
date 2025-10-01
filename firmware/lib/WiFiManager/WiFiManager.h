#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <functional>

class WiFiManager {
public:
    WiFiManager(const String& ssid = "", const String& password = "",
                const String& apName = "ESP32-AP", const String& apPassword = "");

    void begin();
    bool connectSTA(unsigned long timeout = 10000);
    void startAP();                               
    bool isConnected() const;
    String ipAddress() const;

    void setAPStartedCallback(std::function<void()> cb) { _apStartedCallback = cb; }

private:
    String _ssid;
    String _password;
    String _apName;
    String _apPassword;

    std::function<void()> _apStartedCallback;
};
