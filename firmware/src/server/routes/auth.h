#pragma once
#include "Router.h"
#include "JWTAuth.h"
#include "HttpError.h"
#include "HttpSuccess.h"
#include <ConfigManager.h>


// Auth router
Router authRouter("/auth", [](Router *r) {
    r->postWithBody("/login", [r](AsyncWebServerRequest *request, const uint8_t *data) -> HttpSuccess {
        DynamicJsonDocument body(1024);
        DeserializationError error = deserializeJson(body, data);
        if (error) {
            throw HttpError(400, "Invalid JSON body");
        }

        ConfigManager* config = r->use<ConfigManager>("config");
        // Get the password from the JSON body
        String password = body["password"] | "";
        String inputPass = password;
        inputPass.trim();  // removes leading/trailing spaces/newlines
        String hashedPassword = JWTAuth::hmacSha256(inputPass, config->get("jwt")["secret"].as<String>());
        String LOGIN_PASSWORD = config->get("hashedPassword").as<String>();
        Serial.print("Input Hashed Password: ");
        Serial.println(hashedPassword);
        Serial.print("Hashed Saved Password: ");
        Serial.println(LOGIN_PASSWORD);
        if (inputPass.isEmpty()) {
            throw HttpError(400, "Password is required");
        }

        if (hashedPassword != LOGIN_PASSWORD) {
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
