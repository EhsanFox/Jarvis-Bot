#pragma once
#include "Router.h"
#include "../guards/authguard.h"

// Create an instance of AuthGuard
AuthGuard* authGuard = new AuthGuard();

// Info router with some routes secured by AuthGuard
Router infoRouter("/v1", [](Router* r) {
    r->get("/info", [](AsyncWebServerRequest* req) -> String {
        return "{\"version\":\"1.0\"}";
    });

    r->get("/secure-info", [](AsyncWebServerRequest* req) -> String {
        return "{\"secure\":\"This is protected info\"}";
    }, {authGuard});  // Route-level guard
});

// Secure router where all routes are secured by AuthGuard
Router secureRouter("/secure", [](Router* r) {
    r->useGuards({authGuard});  // Router-level guard

    r->get("/data", [](AsyncWebServerRequest* req) -> String {
        return "{\"data\":\"This is secure data\"}";
    });

    r->post("/update", [](AsyncWebServerRequest* req) -> String {
        return "{\"status\":\"Update successful\"}";
    });
});
