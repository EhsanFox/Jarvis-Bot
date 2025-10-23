ServerManager (lib/ServerManager)
===================================

Purpose and role
-----------------
`ServerManager` is the glue between the in-memory `Router` definitions and the `ESPAsyncWebServer` callbacks that run on the ESP32. It is responsible for:

- Mounting LittleFS and serving static assets (the frontend build under `/web`).
- Registering router endpoints (GET/POST) and wiring middleware and guards.
- Converting exceptions (`HttpError`) into uniform JSON error responses.

Detailed behavior and flow
--------------------------
When `begin()` is called `ServerManager` performs:

1. LittleFS mount: `LittleFS.begin(true)` to mount or format if necessary.
2. Static files: `serveStatic("/", LittleFS, "/web/").setDefaultFile("index.html")` — this serves the frontend.
3. Register routers: for each mounted `Router`, iterate `getEndpoints()` and `postEndpoints()` and register `AsyncWebServer` handlers.

Handler execution model
-----------------------
For a simple GET path with a function handler, `ServerManager` creates a lambda that:

- Builds a `runMiddlewares` function which iterates the global `_middlewares` vector. Each middleware receives `request` and a `next` lambda to continue.
- After middleware, router-level guards (`router->routerGuards()`) and endpoint-level `route.guards` are executed in-order. If any guard returns `false` or throws `HttpError`, the request is terminated.
- The final handler is invoked and expected to return an `HttpSuccess`. If the handler returns an `HttpSuccess` that wraps an `AsyncWebServerResponse*`, that response is sent (useful for setting cookies or custom headers). Otherwise the `HttpSuccess` is serialized to JSON and sent with 200 status.

POST body handlers
------------------
Routes created with `postWithBody()` receive raw body bytes via the `AsyncWebServer` upload callback. `ServerManager` defers execution until the upload completes (index+len == total) and then runs the same middleware/guards/handler pipeline as GET.

Error handling specifics
-----------------------
- `HttpError` exceptions are caught and converted into `{ "ok": false, "error": "<message>" }` with the provided status code.
- Standard exceptions and unknown throwables are caught and mapped to 500 with a descriptive message where possible.

Dependency injection
--------------------
`ServerManager` holds a `DependencyContainer _deps`. When `addRouter()` is called the router is given a pointer to this container via `router->attachDependencies(&_deps)` so route handlers can call `r->use<T>("key")` to fetch shared instances (e.g., `ConfigManager`, `WiFiManager`, `Face`).

Examples: registering a router
-----------------------------
```cpp
Router myRouter("/api", [](Router* r){
	r->get("/hello", [](AsyncWebServerRequest* req){
		 DynamicJsonDocument d(64); d["msg"] = "hello"; return HttpSuccess(d);
	});
});
serverManager.addRouter(&myRouter);
```

Security and practical notes
----------------------------
- Static files must be copied into LittleFS (see `postbuild.py`). If `LittleFS.begin()` fails, static serving and some routes may be unavailable.
- Keep middleware cheap and non-blocking. The middleware chain runs in the request callback context — long-running operations should spawn background tasks.

Limitations & caveats
---------------------
- Memory: building JSON responses and holding large `DynamicJsonDocument`s can be heavy; size buffers conservatively and free them promptly.
- Concurrency: `ESPAsyncWebServer` runs callbacks on the internal task(s). If a handler calls blocking OS functions (e.g., long `delay()`), it may block the server.

Testing and debugging
---------------------
- Add unit tests for route handlers by extracting handler logic into small functions which accept `JsonVariant` inputs and return `DynamicJsonDocument` results.
- Use `Serial` logs to confirm endpoint registration and request paths (the code prints registered endpoints on `addRouter`).

Recommended improvements
------------------------
- Add per-route rate limiting and request size guards for hardened production behavior.
- Consider streaming large file responses from LittleFS rather than assembling them into RAM.
