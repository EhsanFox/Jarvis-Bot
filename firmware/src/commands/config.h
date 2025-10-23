#pragma once
#include "Command.h"
#include <Arduino.h>
#include <Utils.h>
#include <ConfigManager.h>
#include <ArduinoJson.h>
#include <vector>

// Directly create a Command instance
Command* configCommand = new Command("config", [](const String& args) -> String {

    std::vector<String> tokens = splitArgs(args);
    if (tokens.empty()) return "[CFG] Usage: config [get|set]";

    String action = tokens[0];

    ConfigManager* config = configCommand->use<ConfigManager>("config");

    if (action == "get") {
        if (tokens.size() < 2) return "[CFG] Usage: config get <variable>";
        
        String key = tokens[1];

        String value = config->get(key).as<String>();
        return "[CFG] Variable [" + key + "]: " + value;
    } 
    else if (action == "set") {
        if (tokens.size() < 4) return "[CFG] Usage: config set <variable> <value> <data-type>";

        String key = tokens[1];
        String dataType = tokens[3];
        String rawValue = tokens[2];
        if (dataType == "string") {
            config->set(key, rawValue);
        } 
        else if (dataType == "number") {
            int value = rawValue.toInt();
            config->set(key, value);
        } 
        else if (dataType == "boolean") {
            bool value = (rawValue == "true" || rawValue == "1");
            config->set(key, value);
        } else return "[CFG] Unknown data type, only [number|string|boolean] is valid";

        return "[CFG] Config value updated.";
    }

    return "[CFG] Unknown config command.";
              
});
