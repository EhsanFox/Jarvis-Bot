#include <Arduino.h>
#include <ConfigManager.h>
#include <WiFiManager.h>
#include <ServerManager.h>
#include <TerminalManager.h>

#include "server/routes/api.h" 
#include "server/middlewares/logger.h"
#include "commands/info.h"

ConfigManager config;
TerminalManager terminal;
WiFiManager *wifiManager = nullptr;
ServerManager *webServer = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Booting...");

    // Setup Terminal Commands
    terminal.addCommand(infoCommand);

    // Setup Wi-Fi manager
    wifiManager = new WiFiManager( config.get("wifi")["ssid"] | "",
                                   config.get("wifi")["password"] | "",
                                   config.get("ap")["name"] | "AI-Bot",
                                   config.get("ap")["password"] | "" );
    wifiManager->begin();

    // Setup webserver
    webServer = new ServerManager(
        config.get("server")["port"] | 80
    );

    // TODO: Add Routers
    webServer->addRouter(&infoRouter);

    // TODO: Add Middlewares
    webServer->use(new LoggerMiddleware());

    wifiManager->setAPStartedCallback([&]() {
        Serial.println("Starting webserver in AP mode...");
        webServer->begin();
    });

    // Try connecting to Wi-Fi from config
    if (!wifiManager->connectSTA(10000)) {
        Serial.println("WiFi connect failed, starting AP...");
        wifiManager->startAP();
    } else {
        Serial.print("Connected to Wi-Fi. IP: ");
        Serial.println(wifiManager->ipAddress());
        webServer->begin(); // start webserver in STA mode
    }
}

void loop() {
    terminal.handleInput();
}
