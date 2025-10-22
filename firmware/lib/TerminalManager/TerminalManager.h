#pragma once
#include <Arduino.h>
#include <vector>
#include "Command.h"
#include "../../include/DependencyContainer.h"

class TerminalManager {
public:
    void addCommand(Command* cmd);
    void handleInput();
    void addDependency(const String& key, void* instance) {
        _deps.set<void>(key, instance);
    }
    DependencyContainer* dependencies() { return &_deps; }

private:
    std::vector<Command*> _commands;
    String _buffer;
    DependencyContainer _deps;
    
    void processLine(const String& line);
};
