#pragma once
#include "Router.h"
#include "JWTAuth.h"
#include "HttpError.h"
#include "HttpSuccess.h"
#include "ConfigManager.h"

Router statusRouter("/status", [](Router *r) {
    r->get("/wizard", [](AsyncWebServerRequest *request) -> HttpSuccess {
        bool isReady = ConfigManager::getInstance().get("isReady");
        return HttpSuccess("Ok.");
    }); // No guards for status route
});
