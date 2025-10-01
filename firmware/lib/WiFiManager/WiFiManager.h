#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <functional>
#include <vector>
#include "ConfigManager.h" // your ConfigManager header; must define ConfigManager and WiFiConfig

class WiFiManager {
public:
    WiFiManager(ConfigManager &cfg);

    // initialize; optional settings in future
    void begin();

    // Scan visible SSIDs. Returns vector of "SSID (rssi dB)" strings.
    std::vector<String> scanSSIDs();

    // Try to connect using config.ssid/config.password.
    // timeoutMs: total ms to wait for connection. returns true if connected.
    bool connectFromConfig(unsigned long timeoutMs = 10000);

    // Start AP with given SSID/password. If pass is empty, AP is open.
    // Calls the apStartedCallback after starting AP.
    bool startAP(const String &apSSID = "AI-Bot-Setup", const String &apPass = "");

    // Register a callback to be executed once AP is up (use to start webserver).
    void setAPStartedCallback(std::function<void()> cb);

    // small helper to check WiFi status
    bool isConnected();

    // returns IP string (station mode) or AP IP if not connected
    String ipAddress();

private:
    ConfigManager &_config;
    std::function<void()> _apStartedCallback = nullptr;
};
