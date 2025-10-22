#pragma once
#include "Router.h"
#include "JWTAuth.h"
#include "HttpError.h"
#include "HttpSuccess.h"
#include "ConfigManager.h"
#include "WiFiManager.h"

Router wifiRouter("/wifi", [](Router *r) {
    r->get("/list", [r](AsyncWebServerRequest *request) -> HttpSuccess {
        WiFiManager* wifi = r->use<WiFiManager>("wifi");
        bool isConnected = wifi->isConnected();
        if (!isConnected) {
            if (wifi->isScanComplete()) {
                String jsonString;
                DynamicJsonDocument wifiList = wifi->getScanResults();
                serializeJson(wifiList, jsonString);
                Serial.println(jsonString);
                return HttpSuccess(wifiList);
            }
            else
            {
                throw HttpError(408, "WiFi Scan not complete, try again later.");
            }
        }
        else {
            throw HttpError(400, "Already connected to a WiFi.");
        }
    });
});
