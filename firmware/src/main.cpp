#include <Arduino.h>
#include "ConfigManager.h"
#include "WiFiManager.h"
#include "ServerManager.h"

#include "server/routes/api.h" 
#include "server/middlewares/logger.h"

ConfigManager config;
WiFiManager *wifiManager = nullptr;
ServerManager *webServer = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Booting...");

    // Load config
    config.begin();

    // Setup Wi-Fi manager
    wifiManager = new WiFiManager(config);
    wifiManager->begin();

    // Setup webserver
    webServer = new ServerManager(80); // default port 80
    webServer->addRouter(&infoRouter); // router imported from api.h

     webServer->use(new LoggerMiddleware());

    // AP started callback
    wifiManager->setAPStartedCallback([&]() {
        Serial.println("Starting webserver in AP mode...");
        webServer->begin();
    });

    // Try connecting to Wi-Fi from config
    if (!wifiManager->connectFromConfig(10000)) {
        Serial.println("WiFi connect failed, starting AP...");
        wifiManager->startAP("AI-Bot-Setup", "");
    } else {
        Serial.print("Connected to Wi-Fi. IP: ");
        Serial.println(wifiManager->ipAddress());
        webServer->begin(); // start webserver in STA mode
    }
}

void loop() {
    static unsigned long last = 0;
    if (millis() - last > 3000) {
        last = millis();
        Serial.print("WiFi connected: ");
        Serial.println(wifiManager->isConnected() ? "YES" : "NO");
        Serial.print("IP: "); Serial.println(wifiManager->ipAddress());
    }
}
