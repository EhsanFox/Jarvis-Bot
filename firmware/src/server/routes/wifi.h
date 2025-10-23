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

    r->postWithBody("/connect", [r](AsyncWebServerRequest *request, const uint8_t *data) -> HttpSuccess {
        DynamicJsonDocument body(1024);
        DeserializationError error = deserializeJson(body, data);
        if (error) {
            throw HttpError(400, "Invalid JSON body");
        }
        WiFiManager* wifi = r->use<WiFiManager>("wifi");
        if (wifi->isConnected()) throw HttpError(400, "Already connected to a WiFi.");
        
        String ssid = body["ssid"];
        String password = body["password"];
        String authPassword = body["authPassword"];

        String canConnect = wifi->tryConnect(ssid, password, 4000);
        if (canConnect == "")
            throw HttpError(400, "Couldn't connect to the wifi.");

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

        ConfigManager* config = r->use<ConfigManager>("config");
        String jwtKey = config->get("jwt")["secret"].as<String>();
        String hashedPassword = JWTAuth::hmacSha256(authPassword, jwtKey);

        JsonVariant wifiJson = config->get("wifi");
        wifiJson["ssid"] = ssid;
        wifiJson["password"] = password;
        config->set("hashedPassword", hashedPassword);
        config->set("wifi", wifiJson);
        config->set("isReady", true);

        DynamicJsonDocument payload(256);
        payload["user"] = "admin";
        payload["iat"] = millis() / 1000; // Issued at time
        payload["exp"] = (millis() / 1000) + 3600; // Expiration time (1 hour)

        String accessToken = JWTAuth::createToken(payload.as<JsonObject>());

        DynamicJsonDocument result(52);
        result["ip"] = canConnect;
        result["accessToken"] = accessToken;
        return HttpSuccess(result);
    });
});
