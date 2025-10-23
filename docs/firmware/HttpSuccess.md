HttpSuccess (include/HttpSuccess.h)
====================================

Purpose
-------
`HttpSuccess` is the standard wrapper for successful HTTP handler responses. `ServerManager` uses it to serialize a consistent JSON envelope to the client: `{ ok: true, data: ... }`, plus optional headers when an `AsyncWebServerResponse*` is provided.

Constructors & payload types
----------------------------
- `HttpSuccess(const String& text)` — simple text response placed into the `data` field.
- `HttpSuccess(bool value)` — boolean success values.
- `HttpSuccess(const DynamicJsonDocument& doc)` — structured JSON payload; preferred for API responses.
- `HttpSuccess(AsyncWebServerResponse* res)` — use a pre-built response to carry custom headers (e.g., cookies). `ServerManager` will use this response as-is.

Behavior
--------
- When a handler returns `HttpSuccess`, `ServerManager` serializes it as `{ ok: true, data: <payload> }` and sets HTTP status code 200 unless the provided `AsyncWebServerResponse*` sets otherwise.

Examples
--------
1) Structured response:

```cpp
DynamicJsonDocument doc(256);
doc["uptime"] = millis();
return HttpSuccess(doc);
```

2) Return a cookie via custom response:

```cpp
auto res = request->beginResponse(200, "application/json", "{\"ok\":true}\n");
res->addHeader("Set-Cookie", String("token=") + token + "; HttpOnly; Path=/");
return HttpSuccess(res);
```

Performance and memory
----------------------
- Avoid creating very large `DynamicJsonDocument` on the stack inside handlers. Prefer heap-allocated documents or reuse documents where appropriate.

Testing
-------
- Unit tests: verify handlers produce correctly structured `HttpSuccess` payloads. When using custom `AsyncWebServerResponse*`, assert headers are preserved.

Interop notes
------------
- Clients expect `{ ok: true, data: ... }` for all successful endpoints. Maintain this format for consistent parsing on the UI side.

Cross-references
----------------
- See `docs/firmware/HttpError.md` for the error envelope used by exception paths.
