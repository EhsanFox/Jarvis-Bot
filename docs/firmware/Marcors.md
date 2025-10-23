Marcors (include/Marcors.h)
=================================

Purpose
-------
`Marcors.h` provides small macros used throughout `src/server/routes` to make the router declarations concise and readable. They are convenience wrappers around `Router` APIs and help keep route files compact and focused on business logic.

Common macros and semantics
---------------------------
- `ROUTER(basePath, setup)` — declare a `Router` with the given `basePath` and run `setup(router*)` to register route handlers.
- `GET(path, handler)` / `POST(path, handler)` — shorthand for `router->get()` / `router->post()` used inside router setup lambdas.
- `USE_MW(mw)` / `USE_GUARD(g)` — attach middleware or guards to the router being constructed.

Examples
--------
Typical usage in a route file:

```cpp
ROUTER("/wifi", [](Router* r){
	USE_MW(loggerMiddleware);
	GET("/list", [](AsyncWebServerRequest* req){
		return HttpSuccess(wifi->getScanResults());
	});
	POST("/connect", [](AsyncWebServerRequest* req){
		// body parsing and connect
	});
});
```

Why macros?
-----------
- Macros reduce boilerplate in small embedded projects where clarity and concise route definitions are desirable.
- They avoid repeating `router->` and let route files resemble declarative route specifications.

Drawbacks and alternatives
-------------------------
- Macros hide function boundaries and make debugging preprocessor expansions slightly harder. If the project grows, consider replacing macros with small inline helper functions or a DSL-like builder API that is type-safe and IDE-friendly.

Cross-references
----------------
- See `docs/firmware/Router.md` for the concrete `Router` API these macros wrap.
