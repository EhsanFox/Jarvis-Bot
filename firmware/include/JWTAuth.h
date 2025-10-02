#pragma once
#include <ArduinoJson.h>
#include <base64.h>
#include "../lib/Crypto/SHA256.h" // Include custom SHA256 implementation
#include "../lib/ConfigManager/ConfigManager.h" // Include ConfigManager for secret key

class JWTAuth {
public:
    // Create a JWT token (access or refresh)
    static String createToken(const JsonObject& payload) {
        String secretKey = ConfigManager::getInstance().get("jwt")["secret"] | "";
        if (secretKey.isEmpty()) return "";

        // Header
        DynamicJsonDocument headerDoc(256);
        headerDoc["alg"] = "HS256";
        headerDoc["typ"] = "JWT";
        String header;
        serializeJson(headerDoc, header);

        // Payload
        String payloadStr;
        serializeJson(payload, payloadStr);

        // Encode header and payload
        String encodedHeader = base64::encode(header);
        String encodedPayload = base64::encode(payloadStr);

        // Create signature
        String signature = hmacSha256(encodedHeader + "." + encodedPayload, secretKey);

        // Return the full token
        return encodedHeader + "." + encodedPayload + "." + signature;
    }

    // Validate a JWT token
    static bool validateToken(const String& token) {
        String secretKey = ConfigManager::getInstance().get("jwt")["secret"] | "";
        if (secretKey.isEmpty()) return false;

        int firstDot = token.indexOf('.');
        int secondDot = token.indexOf('.', firstDot + 1);

        if (firstDot == -1 || secondDot == -1) return false;

        // Extract parts
        String encodedHeader = token.substring(0, firstDot);
        String encodedPayload = token.substring(firstDot + 1, secondDot);
        String providedSignature = token.substring(secondDot + 1);

        // Recreate signature
        String recreatedSignature = hmacSha256(encodedHeader + "." + encodedPayload, secretKey);

        // Compare signatures
        return providedSignature == recreatedSignature;
    }

private:
    // HMAC-SHA256 implementation
    static String hmacSha256(const String& message, const String& key) {
        uint8_t hash[32];
        SHA256 sha256;
        sha256.resetHMAC(reinterpret_cast<const uint8_t*>(key.c_str()), key.length());
        sha256.update(reinterpret_cast<const uint8_t*>(message.c_str()), message.length());
        sha256.finalizeHMAC(reinterpret_cast<const uint8_t*>(key.c_str()), key.length(), hash, sizeof(hash));

        String result;
        for (int i = 0; i < 32; i++) {
            if (hash[i] < 16) result += "0";
            result += String(hash[i], HEX);
        }
        return result;
    }
};
