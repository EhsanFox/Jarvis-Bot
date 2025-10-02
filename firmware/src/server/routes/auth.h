#pragma once
#include "Router.h"
#include "JWTAuth.h"
#include "HttpError.h"
#include "HttpSuccess.h"

// Define the correct password for login
const String LOGIN_PASSWORD = "supersecret";

// Auth router
Router authRouter("/auth", [](Router* r) {
    r->post("/login", [](AsyncWebServerRequest* req) -> String {
        if (!req->hasParam("password", true)) {
            throw HttpError(400, "Password is required");
        }

        // Get the password from the request body
        String password = req->getParam("password", true)->value();

        // Validate the password
        if (password != LOGIN_PASSWORD) {
            throw HttpError(401, "Invalid password");
        }

        // Create a JWT token
        DynamicJsonDocument payload(256);
        payload["user"] = "admin";
        payload["iat"] = millis() / 1000; // Issued at time
        payload["exp"] = (millis() / 1000) + 3600; // Expiration time (1 hour)

        String token = JWTAuth::createToken(payload.as<JsonObject>());

        // Set the token as a cookie
        String responseBody = "{\"ok\": true, \"data\": { \"accessToken\": \"" + token + "\" } }";
        AsyncWebServerResponse* response = req->beginResponse(200, "application/json", responseBody);
        response->addHeader("Set-Cookie", "accessToken=" + token + "; HttpOnly; Path=/");

        // Use HttpSuccess to handle the response
        throw HttpSuccess(responseBody, response);
    });
});
