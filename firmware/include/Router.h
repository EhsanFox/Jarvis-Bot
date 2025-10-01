#pragma once
#include <Arduino.h>
#include <functional>
#include <vector>
#include <ESPAsyncWebServer.h>
#include "Middleware.h"

class Router {
public:
    using RouterSetup = std::function<void(Router*)>;

    // Constructor now supports basePath + optional setup lambda
    Router(const String &basePath = "", RouterSetup setup = nullptr)
        : _basePath(basePath) 
    {
        if (setup) setup(this);
    }

    // Add GET endpoint
    void get(const String& endpoint, std::function<void(AsyncWebServerRequest*)> handler) {
        _getEndpoints.push_back({_basePath + endpoint, handler});
    }

    // Add POST endpoint
    void post(const String& endpoint, std::function<void(AsyncWebServerRequest*)> handler) {
        _postEndpoints.push_back({_basePath + endpoint, handler});
    }

    // Route structure
    struct Route {
        String path;
        std::function<void(AsyncWebServerRequest*)> handler;
    };

    // Accessors
    std::vector<Route> getEndpoints() const { return _getEndpoints; }
    std::vector<Route> postEndpoints() const { return _postEndpoints; }

private:
    String _basePath;
    std::vector<Route> _getEndpoints;
    std::vector<Route> _postEndpoints;
};
