#pragma once
#include <Arduino.h>
#include <functional>
#include <vector>
#include <ESPAsyncWebServer.h>
#include "Middleware.h"
#include "Guard.h"
#include "HttpSuccess.h"
#include "DependencyContainer.h"

class Router {
public:
    using RouterSetup = std::function<void(Router*)>;

    Router(const String &basePath = "", RouterSetup setup = nullptr)
        : _basePath(basePath), _deps(nullptr)
    {
        if (setup) setup(this);
    }

    // Route structure
    struct Route {
        String path;
        std::function<HttpSuccess(AsyncWebServerRequest*)> handler;
        std::function<HttpSuccess(AsyncWebServerRequest*, const uint8_t*)> bodyHandler;
        std::vector<Guard*> guards;
    };

    // --- Dependency Injection Support ---
    void attachDependencies(DependencyContainer* deps) {
        _deps = deps;
    }

    template<typename T>
    T* use(const String& key) const {
        return _deps ? _deps->get<T>(key) : nullptr;
    }

    // --- Endpoint Registration ---

    void get(const String& endpoint,
             std::function<HttpSuccess(AsyncWebServerRequest*)> handler,
             std::vector<Guard*> guards = {}) 
    {
        _getEndpoints.push_back({_basePath + endpoint, handler, nullptr, guards});
    }

    void post(const String& endpoint,
              std::function<HttpSuccess(AsyncWebServerRequest*)> handler,
              std::vector<Guard*> guards = {}) 
    {
        _postEndpoints.push_back({_basePath + endpoint, handler, nullptr, guards});
    }

    void postWithBody(const String& endpoint,
                      std::function<HttpSuccess(AsyncWebServerRequest*, const uint8_t*)> bodyHandler,
                      std::vector<Guard*> guards = {}) 
    {
        _postEndpoints.push_back({_basePath + endpoint, nullptr, bodyHandler, guards});
    }

    // --- Guards ---
    void useGuards(std::vector<Guard*> guards) {
        _routerGuards.insert(_routerGuards.end(), guards.begin(), guards.end());
    }

    // --- Accessors ---
    std::vector<Route> getEndpoints() const { return _getEndpoints; }
    std::vector<Route> postEndpoints() const { return _postEndpoints; }
    std::vector<Guard*> routerGuards() const { return _routerGuards; }
    String basePath() const { return _basePath; }

private:
    String _basePath;
    std::vector<Route> _getEndpoints;
    std::vector<Route> _postEndpoints;
    std::vector<Guard*> _routerGuards;
    DependencyContainer* _deps;   // ✅ added dependency container pointer
};
