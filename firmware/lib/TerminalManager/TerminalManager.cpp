#include "TerminalManager.h"
#include "../../include/Command.h"
#include "../../include/DependencyContainer.h"

void TerminalManager::addCommand(Command* cmd) {
    _commands.push_back(cmd);
}

void TerminalManager::handleInput() {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\r') continue; // ignore carriage return
        if (c == '\n') {
            processLine(_buffer);
            _buffer = ""; // clear buffer for next command
        } else {
            _buffer += c;
        }
    }
}

void TerminalManager::processLine(const String& line) {
    String input = line;
    input.trim();
    if (input.length() == 0) return;

    // Parse command and optional args
    int spaceIndex = input.indexOf(' ');
    String cmdName = input;
    String args = "";
    if (spaceIndex != -1) {
        cmdName = input.substring(0, spaceIndex);
        args = input.substring(spaceIndex + 1);
    }

    cmdName.toLowerCase();
    // args.toLowerCase();

    bool found = false;
    for (Command* cmd : _commands) {
        String cmdCommand = cmd->getCommand();
        cmdCommand.toLowerCase();
        if (cmdCommand == cmdName) {
            cmd->attachDependencies(&_deps);
            String output = cmd->run(args);
            Serial.println(output);
            found = true;
            break;
        }
    }
    if (!found) {
        Serial.println("Unknown command: " + cmdName);
    }
}
