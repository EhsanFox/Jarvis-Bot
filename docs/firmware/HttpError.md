HttpError (include/HttpError.h)
=================================

Purpose
-------
`HttpError` is the firmware's lightweight exception type for signalling HTTP errors from route handlers, middleware and guards. Throwing an `HttpError` is the recommended way to abort processing and produce a consistent JSON error response.

API
---
- `HttpError(int statusCode, const String& message)` — construct an error with HTTP status code and message.
- Accessors: `int statusCode() const`, `String message() const`.

Semantics and server integration
--------------------------------
- `ServerManager` wraps handler execution in a try/catch; if a `HttpError` is caught the server responds with `{ ok: false, error: <message> }` and the specified status code.
- Other exceptions are typically converted to 500 internal errors; prefer throwing `HttpError` for controlled failures.

Common status codes used in the firmware
---------------------------------------
- 400 — Bad Request (validation failed, missing fields)
- 401 — Unauthorized (missing/invalid auth)
- 403 — Forbidden (valid auth but insufficient privileges)
- 404 — Not Found
- 422 — Unprocessable Entity (semantic validation failed)

Examples
--------
1) Validation failure:

```cpp
if(!req->hasParam("ssid")) throw HttpError(400, "ssid parameter is required");
```

2) Authorization failure in a guard:

```cpp
if(!tokenValid) throw HttpError(401, "Invalid token");
```

Best practices
--------------
- Use `HttpError` for expected client errors and validation problems. Reserve generic exceptions for truly unexpected failures.
- Keep the message short and user-facing. If you need to expose debug details, log them locally instead of returning them to the client.

Testing
-------
- Unit test handlers by invoking them and verifying the server response wrapper when `HttpError` is thrown. Mock the `ServerManager` response path if needed.

Cross-references
----------------
- See `docs/firmware/HttpSuccess.md` for how normal handler responses are wrapped.
- See `docs/firmware/Guard.md` for using `HttpError` inside guards.
