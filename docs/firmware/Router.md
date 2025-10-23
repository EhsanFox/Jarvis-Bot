Router (include/Router.h)
====================================

Overview
--------
`Router` is a lightweight in-memory routing description used to register endpoints, guards and handlers before the `ServerManager` mounts them to `ESPAsyncWebServer`. It acts as a staging area to compose middleware, per-router guards and route handlers in a declarative style.

Key concepts
------------
- Base path: routers can be created with a `basePath` that prefixes all contained endpoints (e.g., `/status`).
- Endpoint descriptors: each GET/POST registration stores the path, handler function and an optional guard list.
- Dependency injection: routers receive a pointer to the `DependencyContainer` so handlers can use `use<T>("key")` to access common services.

Primary API (conceptual)
------------------------
- `Router(const String &basePath = "", RouterSetup setup = nullptr)` — construct a router and optionally run a setup lambda to register endpoints.
- `void get(const String& endpoint, Handler handler, std::vector<Guard*> guards = {})` — register GET endpoint.
- `void post(...)` / `void postWithBody(...)` — register POST endpoints (with or without raw body access).
- `void attachDependencies(DependencyContainer* deps)` — called by `ServerManager` to inject shared services.
- `template<typename T> T* use(const String& key) const` — typed accessor inside handlers.

Mounting behavior
-----------------
When `ServerManager::addRouter()` is called, it performs these steps for each stored endpoint:
1. Create `AsyncWebServer` callbacks for the concrete path (basePath + endpoint).
2. On request, run global middleware (configured on `ServerManager`).
3. Run router-level guards, then route-level guards, in order.
4. If all guards pass, call the handler and convert the returned `HttpSuccess` or `AsyncWebServerResponse*` into a real HTTP response. If a `HttpError` is thrown, the manager converts it into an error response.

Examples
--------
1) Simple router definition:

```cpp
Router statusRouter("/status", [](Router* r){
  r->get("/wizard", [r](AsyncWebServerRequest* req){
    auto cfg = r->use<ConfigManager>("config");
    return HttpSuccess(!cfg->get("isReady").as<bool>());
  });
});
```

2) POST with body handler (pseudo):

```cpp
r->postWithBody("/upload", [](AsyncWebServerRequest* req, const uint8_t* body, size_t len){
  // parse raw body
});
```

Edge cases and pitfalls
----------------------
- Duplicate paths: ensure two routers don't register the same exact path unless intended — the last mounted route may shadow earlier ones.
- Body size: handlers receiving raw body bytes should defensively handle large payloads and protect memory.

Testing
-------
- Unit tests: verify router registration populates endpoint descriptors correctly. Mock `ServerManager` mounting to ensure handlers are invoked with middleware and guards in correct order.

Cross-references
----------------
- See `docs/firmware/Middleware.md` and `docs/firmware/Guard.md` for pipeline components executed around handlers.
- See `docs/firmware/ServerManager.md` for the code that converts the `Router` descriptors into live `AsyncWebServer` routes.
