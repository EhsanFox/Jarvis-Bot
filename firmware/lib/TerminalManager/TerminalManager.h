#pragma once
#include <Arduino.h>
#include <vector>
#include "Command.h"

class TerminalManager {
public:
    void addCommand(Command* cmd);
    void handleInput();

private:
    std::vector<Command*> _commands;
    String _buffer;

    void processLine(const String& line);
};
