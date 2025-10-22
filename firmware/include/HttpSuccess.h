#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class HttpSuccess {
public:
    // Constructors for different input types
    HttpSuccess(const String& message, AsyncWebServerResponse* response = nullptr)
        : _type(Type::STRING), _stringValue(message), _response(response) {}

    HttpSuccess(bool message, AsyncWebServerResponse* response = nullptr)
        : _type(Type::BOOL), _boolValue(message), _response(response) {}

    HttpSuccess(const DynamicJsonDocument& doc, AsyncWebServerResponse* response = nullptr)
        : _type(Type::JSON), _jsonDoc(new DynamicJsonDocument(doc)), _response(response) {}

    // Copy constructor (deep copy for JSON)
    HttpSuccess(const HttpSuccess& other)
        : _type(other._type),
          _stringValue(other._stringValue),
          _boolValue(other._boolValue),
          _jsonDoc(other._jsonDoc ? new DynamicJsonDocument(*other._jsonDoc) : nullptr),
          _response(other._response) {}

    // Destructor
    ~HttpSuccess() {
        if (_jsonDoc) delete _jsonDoc;
    }

    // ✅ Return as JSON document
    DynamicJsonDocument json() const {
        DynamicJsonDocument doc(4096);
        doc["ok"] = true;
        switch (_type) {
            case Type::STRING:
                doc["data"] = _stringValue;
                break;
            case Type::BOOL:
                doc["data"] = _boolValue;
                break;
            case Type::JSON:
                if (_jsonDoc) doc["data"] = *_jsonDoc;
                break;
        }
        return doc;
    }

    // ✅ Return as serialized JSON string
    String output() const {
        DynamicJsonDocument doc = json();
        String output;
        serializeJson(doc, output);
        return output;
    }

    AsyncWebServerResponse* response() const { return _response; }

private:
    enum class Type { STRING, BOOL, JSON } _type;
    String _stringValue;
    bool _boolValue = false;
    DynamicJsonDocument* _jsonDoc = nullptr;
    AsyncWebServerResponse* _response = nullptr;
};
