JWTAuth (include/JWTAuth.h)
=================================

Overview
--------
`JWTAuth` implements a small, intentionally lightweight JWT helper tailored for the firmware. It uses HMAC-SHA256 (via mbedTLS) to sign tokens and exposes creation and validation helpers used by the auth routes and `AuthGuard`.

Behavioral summary
------------------
- Token format: `<base64(header)>.<base64(payload)>.<hex(hmac)>` — note the signature is hex-encoded HMAC-SHA256 rather than the more common base64url.
- Header is fixed to `{ "alg": "HS256", "typ": "JWT" }`.
- `createToken(...)` builds the header/payload and computes the signature using the secret from configuration.
- `validateToken(...)` recomputes the HMAC and compares it to the signature in the token.

Public API
----------
- `static String createToken(const JsonObject& payload)` — create a signed token string.
- `static bool validateToken(const String& token)` — return `true` when signature and structure match and token is not expired (if `exp` claim is present and checked by callers).
- `static String hmacSha256(const String& message, const String& key)` — compute hex HMAC (helper used internally and available for tests).

Design choices and interoperability
----------------------------------
- Hex signature: choosing a hex-encoded HMAC simplifies parsing on embedded platforms (no custom base64url handling required), but it differs from common JWT libraries. When integrating with external services, convert signatures accordingly.
- Claim validation: `validateToken` verifies signature. Higher-level checks (expiration `exp`, issuer `iss`, audience `aud`) may be performed by `AuthGuard` or route logic using the parsed payload.

Security considerations
-----------------------
- Use a high entropy `jwt.secret` and never store it in plaintext in public repos. `config.example.json` provides a placeholder; set a strong value in device config.
- Tokens should be transmitted over TLS when possible (this firmware serves HTTP on local networks without TLS in typical setups). When using the token in cookies, set `HttpOnly` and `Secure` flags when appropriate.
- Use constant-time comparison for signatures if integrating with untrusted inputs. The current implementation uses a basic equality check; consider substituting a timing-attack resistant comparator for production-critical scenarios.

Examples
--------
1) Create a token with expiry claim:

```cpp
DynamicJsonDocument p(256);
p["sub"] = "device:123";
p["exp"] = (uint32_t)(time(NULL) + 3600);
String token = JWTAuth::createToken(p.as<JsonObject>());
```

2) Validate and read payload (pseudo):

```cpp
if(!JWTAuth::validateToken(token)) throw HttpError(401, "Invalid token");
JsonObject payload = JWTAuth::parsePayload(token);
if(payload.containsKey("exp") && payload["exp"] < now) throw HttpError(401, "Token expired");
```

Testing
-------
- Unit tests: compute `hmacSha256` with known vectors and verify `createToken` output structure.
- Interop test: generate tokens with this library and verify `AuthGuard` accepts them. If you need to use third-party JWT libraries, implement a small translation layer converting hex signature to base64url.

Cross-references
----------------
- See `docs/firmware/include/Guard.md` and `docs/firmware/src/server/guards/AuthGuard.md` for how tokens are consumed at request time.
