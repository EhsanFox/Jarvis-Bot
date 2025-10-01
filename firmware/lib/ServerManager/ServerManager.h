#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "Router.h"      // include from include/
#include "Middleware.h"  // include from include/
#include <vector>

class ServerManager {
public:
    ServerManager(uint16_t port = 80);

    void use(Middleware* mw);              // global middleware
    void addRouter(Router* router);        // attach a router
    void begin();                           // start the server

private:
    AsyncWebServer _server;
    std::vector<Middleware*> _middlewares;
    std::vector<Router*> _routers;

    void handleRequest(AsyncWebServerRequest* request, std::function<void()> next);
};
