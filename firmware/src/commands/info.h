#pragma once
#include "Command.h"
#include <Arduino.h>
#include <ESP.h>

// Directly create a Command instance
Command* infoCommand = new Command("info", [](const String& args) -> String {
    return "ESP32 Status: FreeHeap=" + String(ESP.getFreeHeap());
});
