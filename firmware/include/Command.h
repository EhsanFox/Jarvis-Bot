#pragma once
#include <Arduino.h>
#include <functional>
#include "DependencyContainer.h"

class Command {
public:
    using CommandHandler = std::function<String(const String& args)>;

    Command(const String& cmdName, CommandHandler handler)
        : _command(cmdName), _handler(handler) {}

    String getCommand() const { return _command; }
    void attachDependencies(DependencyContainer* deps) {
        _deps = deps;
    }

    template<typename T>
    T* use(const String& key) const {
        return _deps ? _deps->get<T>(key) : nullptr;
    }
    String run(const String& args) const { 
        if (_handler) return _handler(args);
        return "";
    }

private:
    String _command;
    CommandHandler _handler;
    DependencyContainer* _deps;
};
