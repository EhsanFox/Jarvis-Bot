#pragma once
#include "Router.h"
#include "JWTAuth.h"
#include "HttpError.h"
#include "HttpSuccess.h"

// Define the correct password for login
const String LOGIN_PASSWORD = "supersecret";

// Auth router
Router authRouter("/auth", [](Router *r) {
    r->postWithBody("/login", [](AsyncWebServerRequest *request, const uint8_t *data) -> HttpSuccess {
        DynamicJsonDocument body(1024);
        DeserializationError error = deserializeJson(body, data);
        if (error) {
            throw HttpError(400, "Invalid JSON body");
        }

        // Get the password from the JSON body
        String password = body["password"] | "";
        String inputPass = password;
        inputPass.trim();  // removes leading/trailing spaces/newlines
        if (inputPass.isEmpty()) {
            throw HttpError(400, "Password is required");
        }

        if (inputPass != LOGIN_PASSWORD) {
            throw HttpError(400, "Invalid password.");
        }

        // Create a JWT token
        DynamicJsonDocument payload(256);
        payload["user"] = "admin";
        payload["iat"] = millis() / 1000; // Issued at time
        payload["exp"] = (millis() / 1000) + 3600; // Expiration time (1 hour)

        String token = JWTAuth::createToken(payload.as<JsonObject>());

        // Set the token as a cookie
        String responseBody = "{\"ok\": true, \"data\": { \"accessToken\": \"" + token + "\" } }";
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", responseBody);
        response->addHeader("Set-Cookie", "accessToken=" + token + "; HttpOnly; Path=/");

        // Send the response
        return HttpSuccess("Access Token Granted.", response);
    }); // Pass an empty vector of guards
});
