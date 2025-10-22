#pragma once
#include "Router.h"
#include "JWTAuth.h"
#include "HttpError.h"
#include "HttpSuccess.h"
#include "ConfigManager.h"

Router statusRouter("/status", [](Router *r) {
    r->get("/wizard", [r](AsyncWebServerRequest *request) -> HttpSuccess {
        ConfigManager* config = r->use<ConfigManager>("config");
        bool isReady = config->get("isReady");
        return HttpSuccess(isReady);
    }); // No guards for status route
});
