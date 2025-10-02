#pragma once
#include <Arduino.h>
#include <functional>
#include <vector>
#include <ESPAsyncWebServer.h>
#include "Middleware.h"
#include "Guard.h"

class Router {
public:
    using RouterSetup = std::function<void(Router*)>;

    Router(const String &basePath = "", RouterSetup setup = nullptr)
        : _basePath(basePath) 
    {
        if (setup) setup(this);
    }

    // Route structure
    struct Route {
        String path;
        std::function<String(AsyncWebServerRequest*)> handler;
        std::vector<Guard*> guards;   // ✅ per-route guards
    };

    // Add GET endpoint
    void get(const String& endpoint,
             std::function<String(AsyncWebServerRequest*)> handler,
             std::vector<Guard*> guards = {}) 
    {
        _getEndpoints.push_back({_basePath + endpoint, handler, guards});
    }

    // Add POST endpoint
    void post(const String& endpoint,
              std::function<String(AsyncWebServerRequest*)> handler,
              std::vector<Guard*> guards = {}) 
    {
        _postEndpoints.push_back({_basePath + endpoint, handler, guards});
    }

    // Add router-wide guards
    void useGuards(std::vector<Guard*> guards) {
        _routerGuards.insert(_routerGuards.end(), guards.begin(), guards.end());
    }

    // Accessors
    std::vector<Route> getEndpoints() const { return _getEndpoints; }
    std::vector<Route> postEndpoints() const { return _postEndpoints; }
    std::vector<Guard*> routerGuards() const { return _routerGuards; }

    String basePath() const { return _basePath;  }

private:
    String _basePath;
    std::vector<Route> _getEndpoints;
    std::vector<Route> _postEndpoints;
    std::vector<Guard*> _routerGuards;   // ✅ global guards
};
