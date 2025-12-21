#pragma once
#include "Command.h"
#include <Arduino.h>
#include <Utils.h>
#include <LittleFS.h>
#include <vector>

// Bash command following the same format as configCommand
Command* bashCommand = new Command("bash", [](const String& args) -> String {
    std::vector<String> tokens = splitArgs(args);
    if (tokens.empty()) return "[BASH] Usage: bash <command> [args]\nType 'bash help' for available commands";
    
    String command = tokens[0];
    static String currentDir = "/";  // Static to maintain state between calls
    
    // Helper function to normalize paths
    auto normalizePath = [&currentDir](const String& path) -> String {
        if (path.startsWith("/")) return path;
        if (path == ".") return currentDir;
        if (path == "..") {
            // Go up one directory
            int lastSlash = currentDir.lastIndexOf('/', currentDir.length() - 2);
            if (lastSlash >= 0) {
                return currentDir.substring(0, lastSlash + 1);
            }
            return "/";
        }
        
        String result = currentDir;
        if (!result.endsWith("/")) result += "/";
        result += path;
        return result;
    };
    
    // pwd - print working directory
    if (command == "pwd") {
        return "[BASH] " + currentDir;
    }
    
    // ls - list directory
    else if (command == "ls") {
        String path = currentDir;
        bool longFormat = false;
        bool showHidden = false;
        
        // Parse flags
        for (size_t i = 1; i < tokens.size(); i++) {
            if (tokens[i] == "-l") {
                longFormat = true;
            } else if (tokens[i] == "-a") {
                showHidden = true;
            } else if (tokens[i] == "-la" || tokens[i] == "-al") {
                longFormat = true;
                showHidden = true;
            } else if (!tokens[i].startsWith("-")) {
                path = normalizePath(tokens[i]);
            }
        }
        
        File dir = LittleFS.open(path);
        if (!dir || !dir.isDirectory()) {
            return "[BASH] Error: Cannot open directory '" + path + "'";
        }
        
        String output = "[BASH] Directory: " + path + "\n";
        File file = dir.openNextFile();
        int count = 0;
        
        while (file) {
            String name = file.name();
            // Extract just the filename
            int lastSlash = name.lastIndexOf('/');
            String displayName = name;
            if (lastSlash >= 0) {
                displayName = name.substring(lastSlash + 1);
            }
            
            // Skip hidden files unless -a flag
            if (!showHidden && displayName.length() > 0 && displayName[0] == '.') {
                file = dir.openNextFile();
                continue;
            }
            
            if (longFormat) {
                output += file.isDirectory() ? "d" : "-";
                output += "rwx ";  // Simplified permissions for LittleFS
                output += String(file.size());
                output += " ";
            }
            
            output += displayName;
            if (file.isDirectory()) output += "/";
            output += "\n";
            
            count++;
            file = dir.openNextFile();
        }
        
        dir.close();
        output += "Total: " + String(count) + " items\n";
        return output;
    }
    
    // cd - change directory
    else if (command == "cd") {
        if (tokens.size() < 2) {
            currentDir = "/";
            return "[BASH] Changed to root directory";
        }
        
        String newPath = normalizePath(tokens[1]);
        
        // Special case for root
        if (newPath == "/" || newPath == "") {
            currentDir = "/";
            return "[BASH] Changed to root directory";
        }
        
        // Check if directory exists
        if (LittleFS.exists(newPath)) {
            File dir = LittleFS.open(newPath);
            if (dir && dir.isDirectory()) {
                currentDir = newPath;
                if (!currentDir.endsWith("/") && currentDir != "/") {
                    currentDir += "/";
                }
                dir.close();
                return "[BASH] Changed to '" + currentDir + "'";
            }
            if (dir) dir.close();
        }
        return "[BASH] Error: Directory '" + tokens[1] + "' not found";
    }
    
    // mkdir - make directory
    else if (command == "mkdir") {
        if (tokens.size() < 2) return "[BASH] Usage: bash mkdir <directory>";
        
        String dirPath = normalizePath(tokens[1]);
        
        if (LittleFS.mkdir(dirPath)) {
            return "[BASH] Directory created: " + dirPath;
        } else {
            return "[BASH] Error: Failed to create directory '" + dirPath + "'";
        }
    }
    
    // rm - remove file/directory
    else if (command == "rm") {
        if (tokens.size() < 2) return "[BASH] Usage: bash rm [-rf] <path>";
        
        bool recursive = false;
        bool force = false;
        String targetPath = "";
        
        // Parse options
        for (size_t i = 1; i < tokens.size(); i++) {
            String token = tokens[i];
            if (token == "-rf" || token == "-fr" || token == "-r") {
                recursive = true;
            } else if (token == "-f") {
                force = true;
            } else if (!token.startsWith("-")) {
                targetPath = normalizePath(token);
            }
        }
        
        if (targetPath.isEmpty()) return "[BASH] Error: No target specified";
        
        if (!LittleFS.exists(targetPath)) {
            return force ? "[BASH] (ignored - file not found)" : "[BASH] Error: Path '" + targetPath + "' not found";
        }
        
        File target = LittleFS.open(targetPath);
        bool isDir = target.isDirectory();
        target.close();
        
        if (isDir && !recursive) {
            return "[BASH] Error: '" + targetPath + "' is a directory (use -r to remove)";
        }
        
        bool success;
        if (isDir) {
            success = LittleFS.rmdir(targetPath);
        } else {
            success = LittleFS.remove(targetPath);
        }
        
        if (success) {
            return "[BASH] Removed: " + targetPath;
        } else {
            return "[BASH] Error: Failed to remove '" + targetPath + "'";
        }
    }
    
    // cat - display file contents
    else if (command == "cat") {
        if (tokens.size() < 2) return "[BASH] Usage: bash cat <file>";
        
        String filePath = normalizePath(tokens[1]);
        
        if (!LittleFS.exists(filePath)) {
            return "[BASH] Error: File '" + filePath + "' not found";
        }
        
        File file = LittleFS.open(filePath, "r");
        if (!file) return "[BASH] Error: Cannot open file '" + filePath + "'";
        
        String output = "[BASH] File: " + filePath + " (" + String(file.size()) + " bytes)\n";
        output += "----------------------------------------\n";
        while (file.available()) {
            output += (char)file.read();
        }
        output += "\n----------------------------------------\n";
        file.close();
        
        return output;
    }
    
    // touch - create empty file
    else if (command == "touch") {
        if (tokens.size() < 2) return "[BASH] Usage: bash touch <file>";
        
        String filePath = normalizePath(tokens[1]);
        File file = LittleFS.open(filePath, "w");
        
        if (file) {
            file.close();
            return "[BASH] File created/updated: " + filePath;
        } else {
            return "[BASH] Error: Failed to create file '" + filePath + "'";
        }
    }
    
    // df - disk free (using correct API)
    else if (command == "df") {
        #ifdef ESP32
            // ESP32 LittleFS info
            size_t totalBytes = LittleFS.totalBytes();
            size_t usedBytes = LittleFS.usedBytes();
        #else
            // For other platforms or older API
            FSInfo fsInfo;
            LittleFS.info(fsInfo);
            size_t totalBytes = fsInfo.totalBytes;
            size_t usedBytes = fsInfo.usedBytes;
        #endif
        
        String output = "[BASH] Filesystem Info (LittleFS):\n";
        output += "Total space: " + String(totalBytes) + " bytes\n";
        output += "Used space: " + String(usedBytes) + " bytes\n";
        output += "Free space: " + String(totalBytes - usedBytes) + " bytes\n";
        output += "Usage: " + String((usedBytes * 100) / totalBytes) + "%\n";
        
        return output;
    }
    
    // help - show help
    else if (command == "help") {
        return "[BASH] Available commands:\n"
               "  pwd                    - Print working directory\n"
               "  ls [-l] [-a] [path]    - List directory\n"
               "  cd <path>              - Change directory\n"
               "  mkdir <dir>            - Create directory\n"
               "  rm [-r] [-f] <path>    - Remove file/directory\n"
               "  cat <file>             - Display file contents\n"
               "  touch <file>           - Create/update file\n"
               "  df                     - Show disk usage\n"
               "  help                   - Show this help\n"
               "\nExamples:\n"
               "  bash ls -la /config\n"
               "  bash cd /data\n"
               "  bash mkdir logs\n"
               "  bash rm -rf /old_logs\n"
               "  bash cat settings.json";
    }
    
    return "[BASH] Unknown command: " + command + "\nType 'bash help' for available commands";
});