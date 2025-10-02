#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class HttpSuccess {
public:
    HttpSuccess(const String& message, AsyncWebServerResponse* response = nullptr)
        : _message(message), _response(response) {}

    String message() const { return _message; }
    AsyncWebServerResponse* response() const { return _response; }

private:
    String _message;
    AsyncWebServerResponse* _response;
};
