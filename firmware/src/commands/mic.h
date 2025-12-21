#pragma once
#include "Command.h"
#include <Arduino.h>
#include <Utils.h>
#include <LittleFS.h>
#include <vector>
#include <MicManager.h>

// Mic command with sub-commands
Command* micCommand = new Command("mic", [](const String& args) -> String {
    std::vector<String> tokens = splitArgs(args);
    if (tokens.empty()) return "[MIC] Usage: mic <command> [args]\nCommands: status, record";
    
    String command = tokens[0];
    
    // Get MicManager instance
    MicManager* micManager = micCommand->use<MicManager>("mic");
    
    if (!micManager) {
        return "[MIC] Error: MicManager not initialized";
    }
    
    // mic status - show recording status
    if (command == "status") {
        String output = "[MIC] Status:\n";
        output += "  Recording: " + String(micManager->isRecording() ? "ACTIVE" : "INACTIVE") + "\n";
        
        if (micManager->isRecording()) {
            output += "  Duration: " + String(micManager->getRecordedDuration()) + " ms\n";
            output += "  Bytes: " + String(micManager->getRecordedBytes()) + "\n";
        }
        
        output += "  Sample rate: 16000 Hz\n";
        output += "  Format: 16-bit mono WAV\n";
        return output;
    }
    
    // mic record - recording control
    else if (command == "record") {
        if (tokens.size() < 2) {
            return "[MIC] Usage: mic record <sub-command>\n"
                   "Sub-commands:\n"
                   "  start <filename.wav>  - Start recording\n"
                   "  stop                  - Stop recording";
        }
        
        String subCommand = tokens[1];
        
        // mic record start <filename>
        if (subCommand == "start") {
            if (tokens.size() < 3) {
                return "[MIC] Usage: mic record start <filename.wav>";
            }
            
            String filename = tokens[2];
            
            // Validate filename
            if (!filename.endsWith(".wav")) {
                filename += ".wav";
            }
            
            if (micManager->isRecording()) {
                return "[MIC] Error: Already recording. Stop first with 'mic record stop'";
            }
            
            // Set up callback for recording status (optional)
            micManager->setRecordingCallback([](uint32_t duration, size_t bytes, float db) {
                // Optional: Uncomment to see live status during recording
                // Serial.printf("[MIC] Recording: %ds, %dKB, Level: %.1f dB\n", 
                //               duration/1000, bytes/1024, db);
            });
            
            if (micManager->startRecording(filename)) {
                return "[MIC] Recording started:\n"
                       "  File: " + filename + "\n" +
                       "  Format: 16000Hz, 16-bit mono WAV\n" +
                       "  Stop with: mic record stop\n" +
                       "  Check status: mic status";
            } else {
                return "[MIC] Error: Failed to start recording\nCheck if filename is valid.";
            }
        }
        
        // mic record stop
        else if (subCommand == "stop") {
            if (!micManager->isRecording()) {
                return "[MIC] Error: Not currently recording";
            }
            
            // Stop recording
            micManager->stopRecording();
            
            // Get recording stats
            uint32_t duration = micManager->getRecordedDuration();
            size_t bytes = micManager->getRecordedBytes();
            float sizeKB = bytes / 1024.0;
            float durationSec = duration / 1000.0;
            
            String output = "[MIC] Recording stopped:\n";
            output += "  Duration: " + String(duration) + " ms (" + String(durationSec, 1) + "s)\n";
            output += "  File size: " + String(bytes) + " bytes (" + String(sizeKB, 1) + " KB)\n";
            output += "  Bitrate: " + String((bytes * 8) / durationSec) + " bps\n";
            output += "  Use 'bash ls' to see the file";
            
            return output;
        }
        
        else {
            return "[MIC] Error: Unknown record sub-command: " + subCommand + "\n" +
                   "Valid sub-commands: start, stop";
        }
    }
    
    // mic help - show help
    else if (command == "help") {
        return "[MIC] Available commands:\n"
               "  mic status                    - Show microphone status\n"
               "  mic record start <filename>   - Start recording to file\n"
               "  mic record stop               - Stop recording\n"
               "  mic help                      - Show this help\n"
               "\nExamples:\n"
               "  mic record start audio.wav\n"
               "  mic status\n"
               "  mic record stop\n"
               "  bash ls *.wav                 # List audio files";
    }
    
    return "[MIC] Unknown command: " + command + "\nType 'mic help' for available commands";
});