#pragma once
#include <Arduino.h>
#include <functional>

class Command {
public:
    using CommandHandler = std::function<String(const String& args)>;

    Command(const String& cmdName, CommandHandler handler)
        : _command(cmdName), _handler(handler) {}

    String getCommand() const { return _command; }
    String run(const String& args) const { 
        if (_handler) return _handler(args);
        return "";
    }

private:
    String _command;
    CommandHandler _handler;
};
