Command (include/Command.h)
================================

Overview
--------
`Command` is the firmware's serial/CLI command abstraction. Each `Command` represents one textual command (for example `info`, `wifi`, `config`) and contains:
- a command name (string)
- a handler callback that takes an argument string and returns a `String` response
- a pointer to an optional `DependencyContainer` used by the handler to access shared services

This pattern centralizes CLI behavior and keeps command registration and execution simple for `TerminalManager`.

Public API (conceptual)
-----------------------
- Constructor: `Command(const String& cmdName, CommandHandler handler)` — create a named command with a handler.
- `String getCommand() const` — return the command name.
- `void attachDependencies(DependencyContainer* deps)` — provide the container used by `use<T>()` lookups.
- `template<typename T> T* use(const String& key) const` — typed accessor that casts the stored void* to `T*`. Returns `nullptr` if missing.
- `String run(const String& args) const` — execute the handler callback. The return value is forwarded to the caller (displayed on serial or sent to the caller UI).

Handler signature and expectations
---------------------------------
Handlers are `std::function<String(const String&)>` (aliased as `CommandHandler`). Expectations for handlers:
- Keep them quick and non-blocking. If long work is required, a handler should schedule it on a task or background timer and return an immediate acknowledgement.
- Validate and sanitize input. Handlers receive a raw argument string; they should parse and validate parameters (e.g., numeric ranges, JSON payloads).
- Use `use<T>("key")` to request shared services (ensure dependencies were attached). Example keys: `"config"`, `"wifi"`, `"face"`.

Examples
--------
1) Registering a command in `main.cpp` or a commands file:

```cpp
auto *infoCmd = new Command("info", [](const String& args){
	// Use attached ConfigManager to build a response
	auto cfg = Command::use<ConfigManager>("config");
	if(!cfg) return String("config missing");
	return String("version: ") + cfg->get("version").as<String>();
});
terminal.addCommand(infoCmd);
```

2) Handler scheduling background work:

```cpp
new Command("reboot", [](const String& args){
	// schedule reboot and return immediately
	xTaskCreate([](void*){ delay(500); ESP.restart(); }, "reboot", 2048, nullptr, 1, nullptr);
	return String("scheduled reboot");
});
```

Error handling and return conventions
------------------------------------
- Handlers should return human-readable messages for CLI use. For machine consumption, consider returning JSON strings.
- Avoid throwing exceptions from handlers; prefer returning error strings and logging details to `TerminalManager`.

Thread-safety and reentrancy
---------------------------
- `Command::run()` is typically invoked from the main loop or the `TerminalManager`'s serial callback. If a handler modifies shared state, it must use appropriate synchronization primitives (mutexes) if the same state can be accessed from other tasks.

Testing
-------
- Unit test strategy: create a fake `DependencyContainer` and inject mock services, then call `run()` with sample args and assert the returned string.
- CLI integration tests: automate serial interactions (e.g., using a script that opens the serial port) to ensure commands produce expected output.

Cross-references
----------------
- See `docs/firmware/TerminalManager.md` for how commands are registered and invoked.
- Use `docs/firmware/DependencyContainer.md` to understand dependency lookups.

Suggestions for improvement
---------------------------
- Add an optional structured response type (e.g., a small struct with status/code/body) so CLI and remote callers can easily parse responses.
- Add automatic help text for each command and a `help` command generator that inspects registered commands.
