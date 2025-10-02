#pragma once
#include <Arduino.h>

class HttpError {
public:
    HttpError(int statusCode, const String& message)
        : _statusCode(statusCode), _message(message) {}

    int statusCode() const { return _statusCode; }
    String message() const { return _message; }

private:
    int _statusCode;
    String _message;
};
