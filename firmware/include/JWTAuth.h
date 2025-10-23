#pragma once
#include <ArduinoJson.h>
#include <base64.h>
#include <mbedtls/md.h> // Include mbedTLS for HMAC-SHA256
#include "ConfigManager.h" // Include ConfigManager for secret key

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
    };
    
    // HMAC-SHA256 implementation using mbedTLS
    static String hmacSha256(const String& message, const String& key) {
        unsigned char hash[32];
        mbedtls_md_context_t ctx;
        const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, info, 1);
        mbedtls_md_hmac_starts(&ctx, reinterpret_cast<const unsigned char*>(key.c_str()), key.length());
        mbedtls_md_hmac_update(&ctx, reinterpret_cast<const unsigned char*>(message.c_str()), message.length());
        mbedtls_md_hmac_finish(&ctx, hash);
        mbedtls_md_free(&ctx);

        String result;
        for (int i = 0; i < 32; i++) {
            if (hash[i] < 16) result += "0";
            result += String(hash[i], HEX);
        }
        return result;
    }
};
