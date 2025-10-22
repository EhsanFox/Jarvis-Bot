#pragma once
#include "Command.h"
#include <Arduino.h>
#include "Utils.h"
#include <WiFiManager.h>
#include <ArduinoJson.h>

// Directly create a Command instance
Command* wifiCommand = new Command("wifi", [](const String& args) -> String {

    std::vector<String> tokens = splitArgs(args);
    if (tokens.empty()) return "Usage: wifi [connect|disconnect|start-ap|stop-ap|status]";

    String action = tokens[0];

    WiFiManager* wifi = wifiCommand->use<WiFiManager>("wifi");

    if (action == "connect") {
        if (tokens.size() < 3) return "Usage: wifi connect <ssid> <password>";
        // return connect(tokens[1], tokens[2]);
        return "command got.";
    } 
    else if (action == "disconnect") {
        wifi->disconnect();
        return "WiFi disconnected.";
    } 
    else if (action == "start-ap") {
        String ssid = tokens.size() > 1 ? tokens[1] : "ESP32-AP";
        String password = tokens.size() > 2 ? tokens[2] : "";
        // return startAP(ssid, password);
        return "Will start ap";
    } 
    else if (action == "stop-ap") {
        // WiFi.softAPdisconnect(true);
        return "Access point stopped";
    } 
    else if (action == "status") {
        return "What status?";
    }
    else if (action == "list") {
        bool isConnected = wifi->isConnected();
        if (!isConnected) {
            if (wifi->isScanComplete()) {
                String jsonString;
                DynamicJsonDocument wifiList = wifi->getScanResults();
                serializeJson(wifiList, jsonString);
                return jsonString;
            }
            else
            {
                return "WiFi List still loading, please try again later.";
            }
        }
        else {
            return "Already connected to a wifi, please try in AP mode.";
        }
    }

    return "Unknown wifi command.";
              
});
