Middleware (include/Middleware.h)
=================================

Overview
--------
`Middleware` defines the contract for request pipeline extensions that can observe, modify or short-circuit HTTP requests before the final route handler executes. Middlewares are composed in order and each middleware is responsible for calling `next()` to continue processing.

Interface contract
------------------
- Signature: `void handle(AsyncWebServerRequest* request, std::function<void()> next)`.
- Responsibilities:
	- Inspect request metadata (path, headers, params).
	- Optionally mutate the request (attach properties, parse body, add headers).
	- Either call `next()` to continue or send a response / throw an `HttpError` to stop further processing.

Common middleware patterns
--------------------------
- Logging middleware — logs incoming requests and timings (see `LoggerMiddleware`).
- Parsing middleware — pre-parse JSON body or form data and attach parsed object to the request for handlers.
- Metrics / telemetry — increment counters or report metrics asynchronously and call `next()` immediately.

Performance and blocking
------------------------
- Middleware executes synchronously in the request callback. Keep middleware fast; heavy work should be offloaded to background tasks.
- If a middleware performs I/O (e.g., reading a file) that might block, either perform the read asynchronously or cache results to avoid stalling the request pipeline.

Error handling
--------------
- To abort with a client-visible error, throw `HttpError` with an appropriate status/message.
- If a middleware sends its own `AsyncWebServerResponse`, it must not call `next()` afterwards.

Examples
--------
Logging middleware (conceptual):

```cpp
struct LoggerMiddleware : public Middleware {
	void handle(AsyncWebServerRequest* req, std::function<void()> next) override {
		Serial.println(String("[REQ] ") + req->url());
		next();
	}
};
```

Testing
-------
- Unit test middleware by invoking `handle()` with a fake request object and a `next()` lambda that sets a flag. Assert the flag and side-effects.

Cross-references
----------------
- See `docs/firmware/include/Marcors.md` for the macros used to attach middleware when creating routers.
- See `docs/firmware/lib/ServerManager.md` for how middleware is executed for each mounted `Router`.
