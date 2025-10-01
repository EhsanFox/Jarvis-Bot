#pragma once
#include "Router.h"

// Create router instance and define endpoints immediately
Router infoRouter("/v1", [](Router* r) {
    r->get("/info", [](AsyncWebServerRequest* req) -> String {
        return "{\"version\":\"1.0\"}";
    });
});
