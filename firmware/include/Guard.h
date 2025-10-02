#pragma once
#include <ESPAsyncWebServer.h>

class Guard {
public:
    virtual bool canActivate(AsyncWebServerRequest* req) = 0;
    virtual ~Guard() {}
};
