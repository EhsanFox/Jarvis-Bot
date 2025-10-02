#pragma once
#include "Command.h"
#include <Arduino.h>
#include <ESP.h>
#include <WiFi.h> // Include WiFi library for status and IP

// Directly create a Command instance
Command* infoCommand = new Command("info", [](const String& args) -> String {

    String wifiStatus = WiFi.isConnected() ? "Connected" : "Disconnected";
    String ipAddress = WiFi.isConnected() ? WiFi.localIP().toString() : "N/A";

    return "=== Jarvis A.I Status ===\n"
              "Firmware Version: 1.0.0\n"
              "Uptime: " + String(millis() / 1000) + " seconds\n"
              "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n"
              "Flash Size: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB\n"
              "SDK Version: " + String(ESP.getSdkVersion()) + "\n"
              "Wi-Fi Status: " + wifiStatus + "\n"
              "IP Address: " + ipAddress + "\n"
              "=========================\n";
              
});
