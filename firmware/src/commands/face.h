#pragma once
#include "Command.h"
#include <Arduino.h>
#include <Utils.h>
#include <FaceManager.h>
#include <ArduinoJson.h>
#include <vector>

// Directly create a Command instance
Command* faceCommand = new Command("face", [](const String& args) -> String {

    std::vector<String> tokens = splitArgs(args);
    if (tokens.empty()) return "[Face] Usage: face [look|mood]";

    String action = tokens[0];

    Face* face = faceCommand->use<Face>("face");
    Serial.printf("[DEBUG] Face pointer: %p\n", face);
    if (!face) return "[Face] FaceManager not initialized!";

    if (action == "look") {
        if (tokens.size() < 2) return "[Face] Usage: face look [front|up|right|left|bottom]";
        
        String direction = tokens[1];
        if (direction == "left") face->LookLeft();
        else if (direction == "right") face->LookRight();
        else if (direction == "up") face->LookTop();
        else if (direction == "bottom") face->LookBottom();
        else if (direction == "front") face->LookFront();
        else
            return "[Face] Unknown direction to look at.";
        return "[Face] Done.";
    } 
    else if (action == "mood") {
        if (tokens.size() < 2) return "[Face] Usage: face mood [normal|sad|angry|worried|annoyed|awe|focused|frustrated|furious|glee|happy|scared|skeptic|sleepy|squint|surprised|suspicious|unimpressed]";

        String mood = tokens[1];
        if (mood == "normal") face->Expression.GoTo_Normal();
        else if (mood == "sad") face->Expression.GoTo_Sad();
        else if (mood == "angry") face->Expression.GoTo_Angry();
        else if (mood == "worried") face->Expression.GoTo_Worried();
        else if (mood == "annoyed") face->Expression.GoTo_Annoyed();
        else if (mood == "awe") face->Expression.GoTo_Awe();
        else if (mood == "focused") face->Expression.GoTo_Focused();
        else if (mood == "frustrated") face->Expression.GoTo_Frustrated();
        else if (mood == "furious") face->Expression.GoTo_Furious();
        else if (mood == "glee") face->Expression.GoTo_Glee();
        else if (mood == "happy") face->Expression.GoTo_Happy();
        else if (mood == "scared") face->Expression.GoTo_Scared();
        else if (mood == "skeptic") face->Expression.GoTo_Skeptic();
        else if (mood == "sleepy") face->Expression.GoTo_Sleepy();
        else if (mood == "squint") face->Expression.GoTo_Squint();
        else if (mood == "surprised") face->Expression.GoTo_Surprised();
        else if (mood == "suspicious") face->Expression.GoTo_Suspicious();
        else if (mood == "unimpressed") face->Expression.GoTo_Unimpressed();
        else return "[Face] Unknown mood.";

        return "[Face] Mood has been changed.";
    }

    return "[Face] Unknown config command.";
              
});
