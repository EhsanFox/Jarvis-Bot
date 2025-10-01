#pragma once
#include "Router.h"

// Create router instance and define endpoints immediately
Router infoRouter("/v1", [](Router* r) {
    r->get("/info", [](AsyncWebServerRequest* req){
        req->send(200, "application/json", "{\"status\":\"ok\", \"version\":\"1.0\"}");
    });
});
