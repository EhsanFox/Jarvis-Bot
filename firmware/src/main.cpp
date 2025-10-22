#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "Displays.h"

#include <ConfigManager.h>
#include <WiFiManager.h>
#include <ServerManager.h>
#include <TerminalManager.h>
#include <FaceManager.h>

#include "server/middlewares/logger.h"

#include "server/routes/auth.h" 
#include "server/routes/status.h"
#include "server/routes/wifi.h"

#include "commands/info.h"
#include "commands/wifi.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Some boards don’t need this
#define SDA_PIN 21
#define SCL_PIN 22

ConfigManager config;
TerminalManager terminal;
WiFiManager *wifiManager = nullptr;
ServerManager *webServer = nullptr;

// GLOBALS
Face *face;

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(115200);
    delay(1000);
    Serial.println("Booting...");
    
    face = new Face(SCREEN_WIDTH, SCREEN_HEIGHT, /* eyeSize = */ 40);
    // Assign the current expression
    face->Expression.GoTo_Normal();

    // Assign a weight to each emotion
    //face->Behavior.SetEmotion(eEmotions::Normal, 1.0);
    //face->Behavior.SetEmotion(eEmotions::Angry, 1.0);
    //face->Behavior.SetEmotion(eEmotions::Sad, 1.0);

    // Automatically switch between behaviours (selecting new behaviour randomly based on the weight assigned to each emotion)
    face->RandomBehavior = true;

    // Automatically blink
    face->RandomBlink = true;

    // Set blink rate
    face->Blink.Timer.SetIntervalMillis(4000);

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
    
    // TODO: Add Dependencies
    webServer->addDependency("wifi", &wifiManager);
    webServer->addDependency("config", &config);
    webServer->addDependency("face", &face);
    
    terminal.addDependency("wifi", &wifiManager);
    terminal.addDependency("config", &config);
    terminal.addDependency("face", &face);

    // TODO: Add Middlewares
    webServer->use(new LoggerMiddleware());
    
    // TODO: Add Routers
    webServer->addRouter(&authRouter);
    webServer->addRouter(&statusRouter);
    webServer->addRouter(&wifiRouter);

    // Setup Terminal Commands
    terminal.addCommand(infoCommand);
    terminal.addCommand(wifiCommand);

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
    face->Update();
}
