Guard (include/Guard.h)
================================

Overview
--------
`Guard` is the firmware's pluggable authorization/validation interface for HTTP routes. Guards are executed before a route handler and decide whether the request may proceed. A guard can either:
- return `true` to allow processing
- return `false` to reject silently
- throw an `HttpError` (preferred for explicit errors) which `ServerManager` converts into an HTTP response

Typical responsibilities
------------------------
- Authentication: verify a JWT or session cookie (`AuthGuard`).
- Authorization: ensure the authenticated subject has required roles/permissions.
- Validation: check request headers or route parameters and throw `HttpError(400, ...)` on invalid input.

Interface contract
------------------
- Expected method: `bool canActivate(AsyncWebServerRequest* request)` or a function object with identical behavior.
- Side effects: prefer to be side-effect free. If a guard must mutate state (e.g., rate-limiting counters), ensure concurrency safety.

Implementation notes
--------------------
- `AuthGuard` verifies the presence and validity of the token (in `Authorization` header or the `token` cookie). It throws `HttpError(401, "Unauthorized")` when missing/invalid.
- Guards run synchronously within the request callback; avoid long-running operations. If an asynchronous check is required, the guard should schedule the check and immediately fail or accept based on cached/known state.

Examples
--------
1) Simple guard that requires a header:

```cpp
struct HeaderGuard : public Guard {
	bool canActivate(AsyncWebServerRequest* req) override {
		if(!req->hasHeader("x-device")) throw HttpError(400, "Missing x-device header");
		return true;
	}
};
```

Error handling and best practices
--------------------------------
- Throw `HttpError` for explicit client facing errors; this makes responses consistent and human-friendly.
- Do not log secrets (e.g., entire JWTs) in guard logs. Log only the minimum needed to debug (token presence, claim names, user id).

Testing
-------
- Unit tests: call the guard's `canActivate` with a fabricated `AsyncWebServerRequest` or a small test wrapper and verify behavior for valid/malformed inputs.
- Integration tests: exercise endpoints with and without valid tokens to ensure guards block/allow correctly and `ServerManager` serializes errors as expected.

Cross-references
----------------
- See `docs/firmware/JWTAuth.md` for token format and signature details used by `AuthGuard`.
- See `docs/firmware/HttpError.md` for how throwing `HttpError` translates to HTTP responses.
