#pragma once
#include "Middleware.h"

class LoggerMiddleware : public Middleware {
public:
    void handle(AsyncWebServerRequest* request, std::function<void()> next) override {
        Serial.print("[Logger] Request URL: ");
        Serial.println(request->url());
        next();
    }
};
