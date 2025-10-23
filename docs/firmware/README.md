Firmware — Jarvis-Bot
=====================

Overview
--------
This document is a comprehensive entry point for the ESP32 firmware in `firmware/`. It explains the project layout, build/upload workflow, configuration, runtime architecture, key integration points (HTTP server, Wi‑Fi, display/face rendering, CLI), troubleshooting notes and pointers to the per-file documentation that lives under `docs/firmware/`.

Target audience
---------------
- Firmware maintainers and contributors who need to change or extend behavior.
- Frontend developers who want to understand how the web UI is served from LittleFS.
- QA/CI engineers who want to automate builds and filesystem uploads.

Repository layout (firmware/)
----------------------------
- `include/` — small shared headers and contracts (DI, Router, Middleware, Command, JWT helper, HttpError/Success wrappers). See `docs/firmware/` for per-header docs.
- `lib/` — modular libraries with clear responsibilities:
  - `ConfigManager/` — persistent JSON config on LittleFS
  - `FaceManager/` — eye drawing, animations, presets and behavior
  - `ServerManager/` — mounts routers, serves static files, handles middleware/guards
  - `TerminalManager/` — serial command lifecycle and command registry
  - `WiFiManager/` — station/AP management and scanning
  - `Utils/` — small helpers (arg splitting, timers)
  See `docs/firmware/` for detailed pages per library.
- `scripts/` — PlatformIO extra scripts: `prebuild.py` (hook wiring) and `postbuild.py` (build frontend and prepare LittleFS content). See `docs/firmware/`.
- `src/` — application wiring:
  - `main.cpp` — constructs services, registers dependencies, mounts routers and starts the server/face loop
  - `commands/` — CLI `Command` instances (info, config, wifi, face)
  - `server/` — routers, guards (AuthGuard), middlewares (Logger), and route handlers

High-level architecture and runtime flow
----------------------------------------
1. Boot & config
	- On boot `main.cpp` mounts LittleFS and uses `ConfigManager` to load `/config.json` (fallback to `config.example.json` if missing).
	- `ConfigManager` exposes dot-separated `get()`/`set()` APIs used throughout the codebase.

2. Dependency injection
	- `DependencyContainer` registers shared instances under string keys (e.g., `"config"`, `"wifi"`, `"face"`). Routers and commands call `attachDependencies()` so handlers can call `use<T>("key")`.

3. Networking
	- `WiFiManager` tries to connect to saved STA credentials. When credentials are missing or connection fails it starts an AP to run the setup wizard.
	- Scanning is asynchronous; the web UI calls `/wifi/list` to discover networks and `/wifi/connect` to attempt connections.

4. HTTP server & routes
	- `ServerManager` mounts `Router` objects defined in `src/server/routes`. Each router holds endpoint descriptors, guards and handlers.
	- Middleware runs first (e.g., `LoggerMiddleware`). Guards (e.g., `AuthGuard`) gate access; handlers return `HttpSuccess` or throw `HttpError` for failure.
	- The frontend static site is served from LittleFS under `/` (populated by `postbuild.py`).

5. Face rendering
	- `FaceManager` keeps a rendering loop that updates two SSD1306 displays using `U8G2` full-frame buffers.
	- Animations (blink, look, transitions) are orchestrated via small helper classes (BlinkAssistant, LookAssistant, EyeTransition, etc.).

6. Terminal / serial CLI
	- `TerminalManager` registers `Command` objects. Commands access services via the dependency container and must be responsive (offload heavy tasks).

Key configuration and files
---------------------------
- `config.example.json` — example structure and defaults. Important keys:
  - `wifi` — `ssid`, `password` for STA mode
  - `ap` — `ssid`, `password` used when device runs setup AP
  - `server.port` — HTTP port
  - `jwt` — `secret`, `expiration`, `issuer` used by `JWTAuth`
  - `face` — defaults for eye presets, brightness, and animations

Build & upload workflow (developer) — Windows (PowerShell) notes
---------------------------------------------------------------
Prerequisites
- PlatformIO CLI installed (the project uses PlatformIO `platformio.ini` for ESP32/Arduino)
- Node.js and npm (for frontend build)

Quick local workflow (from repo root)

```powershell
# Build firmware
platformio run --environment esp32dev
# Prepare filesystem image (ensure frontend is built/copy assets first)
platformio run --environment esp32dev --target buildfs
# Upload filesystem image to device
platformio run --environment esp32dev --target uploadfs
# Upload firmware binary
platformio run --environment esp32dev --target upload
```

Automated workflow (scripts)
- `postbuild.py` (PlatformIO extra script) will build the frontend (`npm run build` in `frontend/`) and copy the static files into `firmware/data/web` so `uploadfs` packages them.
- `prebuild.py` coordinates PlatformIO hook registration so `uploadfs` is run at the right time.

Debugging & troubleshooting
---------------------------
- Serial logs: use `platformio device monitor -b 115200` to view startup logs and errors.
- LittleFS issues: corrupted or missing `config.json` can leave the device in an uninitialized state. Check `data/config.json` or the fallback `config.example.json`.
- Wi‑Fi: if the device consistently fails to connect, verify correct `ssid`/`password` and check AP vs STA mode interactions. Watch for watchdog resets if connection loops block the main thread.
- Display problems: blank or garbled output usually indicates wrong I2C pins or insufficient heap for dual `U8G2` full buffers. Try disabling one display or lowering buffer usage.

Security notes
--------------
- JWT tokens are generated with an HMAC-SHA256 signature (hex string) by `JWTAuth`. This differs from standard base64url signatures—if you integrate external JWT libraries, implement a translation layer.
- Keep `jwt.secret` high-entropy and do not commit it into the repository.
- The firmware serves HTTP (no TLS) in typical local setups; be cautious exposing the device to untrusted networks.

Testing strategy
----------------
- Unit tests: The repo is C++/PlatformIO; most code is embedded and depends on hardware. For logic-heavy modules (config parsing, JWT, utils), extract testable functions and create host-side unit tests using a PlatformIO test environment or a desktop harness where feasible.
- Integration: run the firmware on hardware, use the web UI for the wizard flows and the serial CLI for commands. Capture serial logs for regression checks.

Where to find more detailed docs
--------------------------------
- Includes: `docs/firmware/` — low-level contracts (Router, Middleware, Command, JWTAuth, HttpError/Success, etc.)
- Libraries: `docs/firmware/` — deep dives into `ConfigManager`, `FaceManager`, `ServerManager`, `TerminalManager`, `WiFiManager`, and `Utils` with examples and testing tips.
- Scripts: `docs/firmware/` — `prebuild.py` and `postbuild.py` expanded docs including PowerShell examples and CI guidance.
- Source wiring and endpoints: `docs/firmware/` — detailed application flow (main.cpp), CLI commands and server route descriptions.

