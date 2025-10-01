#pragma once
#include <Arduino.h>
#include <functional>
#include <ESPAsyncWebServer.h>

class Middleware {
public:
    Middleware() = default;
    virtual void handle(AsyncWebServerRequest* request, std::function<void()> next) = 0;
};
