#pragma once
#include "Command.h"
#include <Arduino.h>
#include <Utils.h>
#include <WiFiManager.h>
#include <ConfigManager.h>
#include <ArduinoJson.h>
#include <vector>

// Directly create a Command instance
Command* wifiCommand = new Command("wifi", [](const String& args) -> String {

    std::vector<String> tokens = splitArgs(args);
    if (tokens.empty()) return "[WiFI] Usage: wifi [connect|disconnect|start-ap|stop-ap|status]";

    String action = tokens[0];

    WiFiManager* wifi = wifiCommand->use<WiFiManager>("wifi");
    ConfigManager *config = wifiCommand->use<ConfigManager>("config");

    if (action == "connect") {
        if (tokens.size() < 3) return "[WiFI] Usage: wifi connect <ssid> <password>";
        if (wifi->isConnected()) return "[WiFI] Already connected to a WiFi.";
        
        String ssid = tokens[1];
        String password = tokens[2];

        String canConnect = wifi->tryConnect(ssid, password, 4000);
        if (canConnect == "")
            return "[WiFI] Couldn't connect to the wifi.";

        // Background task to stop AP after X seconds
        unsigned long stopDelayMs = 5000;  // for example: 5 seconds

        xTaskCreate(
            [](void* param) {
                WiFiManager* wifi = static_cast<WiFiManager*>(param);
                delay(5000);  // wait X seconds
                wifi->stopAP();  // stop the access point
                vTaskDelete(NULL);
            },
            "StopAPTask",   // name
            4096,           // stack size
            wifi,           // parameter
            1,              // priority
            NULL            // handle
        );

        JsonVariant wifiJson = config->get("wifi");
        wifiJson["ssid"] = ssid;
        wifiJson["password"] = password;
        config->set("wifi", wifiJson);

        return "[WiFI] Connected to WiFi | IP Address: " + canConnect;
    } 
    else if (action == "disconnect") {
        wifi->disconnect();
        return "[WiFI] WiFi disconnected.";
    } 
    else if (action == "start-ap") {
        String ssid = tokens.size() > 1 ? tokens[1] : config->get("ap")["ssid"].as<String>();
        String password = tokens.size() > 2 ? tokens[2] : config->get("ap")["password"].as<String>();
        // return startAP(ssid, password);
        return "[WiFI] Will start ap";
    }
    else if (action == "status") {
        return "[WiFI] What status?";
    }
    else if (action == "list") {
        bool isConnected = wifi->isConnected();
        if (isConnected) return "[WiFI] Already connected to a wifi, please try in AP mode.";
        else {
            if(!wifi->isScanComplete()) return "[WiFI] WiFi List still loading, please try again later.";
            else {
                String jsonString;
                DynamicJsonDocument wifiList = wifi->getScanResults();
                serializeJson(wifiList, jsonString);
                return "[WiFI] Available list: " + jsonString;
            }
        }
    }

    return "[WiFI] Unknown wifi command.";
              
});
