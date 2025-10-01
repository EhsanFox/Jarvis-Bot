#pragma once
#include <Arduino.h>
#include <functional>
#include <vector>
#include <ESPAsyncWebServer.h>
#include "Middleware.h"

class Router {
public:
    Router(const String &basePath = "") : _basePath(basePath) {}

    void get(const String& endpoint, std::function<void(AsyncWebServerRequest*)> handler) {
        _getEndpoints.push_back({_basePath + endpoint, handler});
    }

    void post(const String& endpoint, std::function<void(AsyncWebServerRequest*)> handler) {
        _postEndpoints.push_back({_basePath + endpoint, handler});
    }

    struct Route {
        String path;
        std::function<void(AsyncWebServerRequest*)> handler;
    };

    std::vector<Route> getEndpoints() const { return _getEndpoints; }
    std::vector<Route> postEndpoints() const { return _postEndpoints; }

private:
    String _basePath;
    std::vector<Route> _getEndpoints;
    std::vector<Route> _postEndpoints;
};
