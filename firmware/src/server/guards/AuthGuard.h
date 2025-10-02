// UserGuard.h
#pragma once
#include "Guard.h"
#include "JWTAuth.h"
#include "HttpError.h"

class AuthGuard : public Guard {
public:
    bool canActivate(AsyncWebServerRequest* request) override {
        String token;

        // Check for Authorization header
        if (request->hasHeader("Authorization")) {
            auto auth = request->getHeader("Authorization")->value();
            if (auth.startsWith("Bearer ")) {
                token = auth.substring(7);  // Extract token after "Bearer "
            }
        }

        // Check for accessToken in cookies if no Authorization header
        if (token.isEmpty() && request->hasHeader("Cookie")) {
            auto cookieHeader = request->getHeader("Cookie")->value();
            token = extractTokenFromCookies(cookieHeader, "accessToken");
        }

        // Validate the token
        if (!token.isEmpty() && JWTAuth::validateToken(token)) {
            return true;
        }

        // Throw HttpError if token is invalid or missing
        throw HttpError(401, "Unauthorized");
    }

private:
    // Helper function to extract a specific token from cookies
    String extractTokenFromCookies(const String& cookieHeader, const String& tokenName) {
        int start = cookieHeader.indexOf(tokenName + "=");
        if (start == -1) return "";

        start += tokenName.length() + 1;  // Move past "tokenName="
        int end = cookieHeader.indexOf(";", start);
        if (end == -1) end = cookieHeader.length();

        return cookieHeader.substring(start, end);
    }
};
