#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "../../include/Router.h"      // include from include/
#include "../../include/Middleware.h"  // include from include/
#include "../../include/DependencyContainer.h"
#include <vector>


class ServerManager {
public:
    ServerManager(uint16_t port = 80);

    void use(Middleware* mw);              // global middleware
    void addDependency(const String& key, void* instance) {
        _deps.set<void>(key, instance);
    }
    DependencyContainer* dependencies() { return &_deps; }
    void addRouter(Router* router);        // attach a router
    void begin();                          // start the server

private:
    AsyncWebServer _server;
    std::vector<Middleware*> _middlewares;
    std::vector<Router*> _routers;
    DependencyContainer _deps;

    void processRequest(AsyncWebServerRequest* request,
                        const Router::Route& route,
                        const std::vector<Guard*>& routerGuards,
                        std::function<void()> next);
};
