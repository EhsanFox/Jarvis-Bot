# Firmware `src/` — Detailed documentation

This document is a comprehensive guide to everything under `firmware/src/`. It explains design goals, public APIs, wiring, runtime flows, command-line/HTTP APIs, common debugging steps, and suggestions for tests and extensions.

Contents
- Overview and goals
- File-by-file explanation
- Dependency injection and keys
- Runtime flows (boot, wifi setup, server, face rendering)
- Commands (serial)
- HTTP routes (server) and expected payloads
- Error handling and edge cases
- Debugging, testing and validation
- Extension points and recommended next steps

## Overview and goals

The `src/` folder contains the main application glue for Jarvis-Bot firmware running on an ESP32. The design favors small, testable libraries under `lib/` with a thin application layer that:

- Wires services together (config, wifi, web server, face manager).
- Exposes a serial CLI for local control and diagnostics.
- Hosts a web UI served from LittleFS and protected by simple JWT-based auth.
- Runs non-blocking animation loops for the face subsystem.

Primary responsibilities of `src/`:
- Initialize and configure hardware (I2C, displays).
- Create service instances and register them in the `DependencyContainer`.
- Add middleware, routers, and start the `ServerManager`.
- Register command handlers with `TerminalManager`.
- Provide a minimal runtime loop that handles serial input and updates the face.

## File-by-file explanation

This section lists the main sources and describes their role and notable behaviors.

### `main.cpp`

Role: application bootstrap and wiring. This file does the heavy lifting of creating and connecting the system components.

Key actions performed in `setup()`:

- Disable brownout reset with `WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);` (board-specific workaround).
- Initialize I2C via `Wire.begin(SDA_PIN, SCL_PIN)` and `Serial.begin(115200)` for logs and CLI.
- Create `Face` instance using `ConfigManager` values (fallback to sensible defaults). Set initial expression to `GoTo_Normal()` and configure blink timers.
- Instantiate `WiFiManager` using values stored in `ConfigManager`.
- Instantiate `ServerManager` and register dependencies (keys: `wifi`, `config`, `face`).
- Register a basic `LoggerMiddleware` (prints request URLs).
- Add routers: `authRouter`, `statusRouter`, `wifiRouter` (these are defined in `src/server/routes/*`).
- Register terminal commands (info, wifi, config, face) and inject dependencies into the terminal manager.
- Try to connect to Wi-Fi using `config` values. If connect fails: `wifiManager->startAP()` and the AP mode will trigger the webserver; otherwise start server in STA mode.

Key runtime loop:

- `terminal.handleInput()` — reads serial, executes commands.
- `face->Update()` — updates all face animations and pushes display buffers.

Important implementation notes
- `webServer->addDependency("wifi", wifiManager);` is how the DI system wires services into route handlers — use exact key names.
- `ConfigManager` is created as a global variable (`ConfigManager config;`) in `main.cpp` and also added to the DI container; this is the canonical persisted configuration.
- The code uses FreeRTOS tasks for a deferred call to stop the AP after a successful wifi connect (see `wifi` command and `wifi` route). These tasks call `vTaskDelete(NULL)` when done.

### `src/commands/` (info.h, wifi.h, config.h, face.h)

Purpose: expose a small shell over Serial using `TerminalManager` and `Command` objects. Each command is created as a `Command*` instance and registered in `main.cpp`.

Common patterns:
- Use `splitArgs()` from `lib/Utils` to parse command-line tokens.
- Use `command->use<T>("key")` to retrieve dependencies (e.g., `wifi`, `config`, `face`) previously registered in `ServerManager` or `TerminalManager`.

Command summaries:
- `info` — returns device diagnostics (uptime, free heap, flash size, SDK version, Wi-Fi status and IP).
- `wifi` — supports `connect`, `disconnect`, `start-ap`, `stop-ap`, `status`, `list`. `connect` updates `ConfigManager` and calls `WiFiManager::tryConnect()`.
- `config` — `get` and `set` operations for persisted config keys (supports `string`, `number`, `boolean`). Keys are dot-separated paths into the JSON config.
- `face` — `look` and `mood` commands to manipulate the face at runtime.

Example: call from serial (115200) to set server port

	config set server.port 8080 number

Tips
- Commands run on the main loop; avoid long blocking operations. Use tasks if you need background work.

### `src/server/` (guards, middlewares, routes)

Purpose: expose HTTP APIs and static assets. The server is bootstrapped via `ServerManager`, which mounts `Router` instances from the `src/server/routes` files.

Files
- `middlewares/logger.h` — logs request URLs to serial and calls `next()`.
- `guards/AuthGuard.h` — extracts Bearer token (or cookie `accessToken`) and validates via `JWTAuth::validateToken`. Throws `HttpError(401, "Unauthorized")` on failure.
- `routes/auth.h` — `POST /auth/login` accepts `{ password }`, compares hashed password (HMAC-SHA256 using `jwt.secret`) and issues a token set as `Set-Cookie: accessToken=<token>; HttpOnly; Path=/` and also returns a JSON response with `accessToken` in `data`.
- `routes/status.h` — `GET /status/wizard` returns whether initial setup is required (uses `config.get("isReady")`).
- `routes/wifi.h` — `GET /wifi/list` returns scan results; `POST /wifi/connect` attempts connection, stores wifi credentials and `hashedPassword` in `ConfigManager`, sets `isReady=true` and returns `{ ip, accessToken }`.

Router behaviors
- Routes return `HttpSuccess` for normal responses or throw `HttpError` for controlled failures. `ServerManager` catches these and returns JSON `{ ok:false, error: <message> }` with the provided status code.
- `Router::postWithBody()` handlers receive `(AsyncWebServerRequest*, const uint8_t* data)` giving raw POST bodies; they are expected to be JSON and handlers use `deserializeJson()`.

Security model
- The initial setup flow sets a `hashedPassword` in `ConfigManager` on `/wifi/connect`. Login uses that hashed password.
- Tokens are HMAC-SHA256 signed strings built by `JWTAuth::createToken` (note: signature is hex, not base64url — both ends must be consistent).

## Dependency injection and keys

The system uses a minimal `DependencyContainer` mapping `String -> void*`. The following keys are used across `src/`:

- `"config"` -> `ConfigManager*`
- `"wifi"` -> `WiFiManager*`
- `"face"` -> `Face*`

Usage rules
- Register dependencies in `main.cpp` before starting the server and before you register command handlers that may use them.
- In a router or command, use `r->use<ConfigManager>("config")` or `cmd->use<WiFiManager>("wifi")` to obtain typed pointers.

## Runtime flows

### Boot / initial setup

1. On power-up `setup()` mounts LittleFS via `ConfigManager` constructor and reads `/config.json`.
2. `Face` object is constructed and `Expression.GoTo_Normal()` sets the neutral face.
3. `WiFiManager` is created with `config.get("wifi.ssid")` and `config.get("wifi.password")`.
4. If `config.get("isReady")` is false, the device attempts to connect using config values; if connection fails or config not ready, it enters AP mode and starts the web server to allow the user to configure Wi‑Fi through the web UI.

### Web-based wizard (setup)

1. User connects to device AP and opens web UI served from LittleFS (`/web/index.html`).
2. Web UI calls `GET /status/wizard`: when `true` UI presents setup flow.
3. Web UI calls `POST /wifi/connect` with `{ ssid, password, authPassword }`.
4. The server attempts to connect using `WiFiManager::tryConnect`. On success the server saves `wifi` object, saves `hashedPassword` computed with `JWTAuth::hmacSha256(authPassword, jwt.secret)`, sets `isReady=true`, and returns `{ ip, accessToken }`.
5. Client stores access token and uses it for subsequent protected requests.

### Normal operation (STA mode)

1. Device connects to configured Wi-Fi, `ServerManager::begin()` is called in STA mode, mounting routers and static assets.
2. The `Face` loop runs continuously (via `loop()`), updating animations and sending buffers to both OLED displays.
3. CLI remains available via Serial for local commands.

## Commands (serial) — details and examples

Common idioms:
- Commands are registered as `Command*` variables in `src/commands/*.h` and added to `TerminalManager` via `terminal.addCommand()`.
- Each command reads tokens with `splitArgs()` and fetches dependencies via `cmd->use<T>("key")`.

Examples
- `info` — prints JSON-like giveaway of system status.
- `wifi connect MySSID MySecret` — attempts to connect, updates `ConfigManager`, and spins a background task to stop the AP after a short delay if connection succeeded.
- `config get face.width` — returns stored config value.
- `face mood happy` — transitions the face to a predefined expression.

Implementation tip
- Commands convert input to lower-case for comparison; be aware that values (like passwords) are lowercased too by the current implementation — if case-preserving passwords are needed, adjust parsing.

## HTTP routes — request/response contracts (examples)

### POST /auth/login
Request body (JSON):
```
{ "password": "<plaintext>" }
```
On success: 200 with Set-Cookie header and JSON `{ ok: true, data: { accessToken: "..." } }`.
On failure: throws `HttpError(400, "Invalid password.")` or `HttpError(400, "Password is required")`.

### GET /status/wizard
Response: `HttpSuccess(!isReady)` — JSON `{ "ok": true, "data": <boolean> }`.

### GET /wifi/list
Response: JSON array of networks (see `WiFiManager::getScanResults()`), or `HttpError(408)` if scan not complete.

### POST /wifi/connect
Request body (JSON):
```
{ "ssid": "MySSID", "password": "secret", "authPassword": "adminpass" }
```
Response: JSON with `{ ip: "<ip-address>", accessToken: "..." }` on success.

Errors: returns `HttpError` on invalid JSON, scan fail or already-connected state.

## Error handling and edge cases

- `HttpError` is used to terminate a request early with a controlled status code and message; `ServerManager` will convert it to `{ ok:false, error: <message> }`.
- Uncaught exceptions produce 500 responses with `Unknown error` or exception message when available.
- If `ConfigManager` fails to mount LittleFS, the firmware logs a message and may not be able to serve static assets; handle this by checking LittleFS logs on boot.
- The JWT implementation stores signature as hex string; if integrating with external services, ensure their JWT implementation expects the same format (or adapt `JWTAuth`).

Edge case checklist
- Ensure `jwt.secret` exists in config before attempting `POST /wifi/connect` (the code expects to hash `authPassword` using the secret).
- If the device fails to connect to Wi-Fi and keeps starting the AP, check `config.get("wifi.ssid")` and `config.get("wifi.password")` for correctness and visibility/escape characters.

## Debugging, testing and validation

Serial logging
- Use `Serial` (115200) to read boot logs. The firmware prints status of routers, middleware registration, LittleFS mounting and connection attempts.

Verifying web UI
- Ensure the frontend build is copied into `firmware/data/web` before uploading filesystem. The `postbuild.py` script tries to run `npm run build` in `../frontend` and copy build output to `data/web`. If you prefer to copy manually, place compiled assets into `firmware/data/web`.

Testing endpoints
- From a laptop on the same network use `curl` or Postman to call `GET /status/wizard`, `GET /wifi/list` and `POST /wifi/connect`. Use the returned `accessToken` as `Authorization: Bearer <token>` for protected endpoints.

Unit / integration tests (suggestions)
- Add PlatformIO unit tests that:
	- Verify `ConfigManager` can `set()` and `get()` nested keys and persist to LittleFS (use test harness or mock filesystem).
	- Mock `WiFi` behavior to test `WiFiManager::tryConnect` flows.
	- Simulate `AsyncWebServerRequest` to test router handlers' responses.

Smoke tests
- Device boots and logs `✅ Webserver started!` when server begins; if not, inspect LittleFS and router registration logs printed by `ServerManager`.

## Extension points and recommended next steps

1. Improve JWT: validate `exp` claim and use base64url signatures to conform with standard JWT libraries if you ever want interoperability.
2. Add tests: small unit tests for `ConfigManager` and CI checks for static code analysis.
3. Add better CLI parsing: support quoted arguments and preserve case for secrets.
4. Improve error messages and logging (include correlation IDs for requests to trace flow across middleware).

## Quick reference: important filenames and responsibilities

- `main.cpp` — application wiring and bootstrap.
- `src/commands/*` — serial CLI commands.
- `src/server/routes/*` — HTTP router definitions.
- `src/server/guards/*` — route guards (AuthGuard).
- `src/server/middlewares/*` — middleware (Logger).

## Final notes

This document is intended to be the authoritative developer guide for the `src/` folder. If you want, I can:

- Convert this into a printed PDF or a single-page HTML index.
- Expand any section with code examples, call graphs, or sequence diagrams.
- Add inline code comments to critical files showing the expected usage and contract.

Tell me which of those follow-ups you'd like next.

