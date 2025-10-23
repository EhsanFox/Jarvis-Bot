DependencyContainer (include/DependencyContainer.h)
===================================================

Purpose
-------
`DependencyContainer` is a minimal service locator used across the firmware to provide shared objects (singletons) to modules that need them. It stores raw pointers (`void*`) under string keys and exposes typed accessors.

Why it exists
--------------
- The firmware is modular (commands, routers, face rendering, Wi-Fi). Rather than passing long parameter lists, the container lets the system register shared services in `main.cpp` and inject them into commands and routers at runtime.

Core API
--------
- `template<typename T> void set(const String& key, T* instance)` — register `instance` under `key`.
- `template<typename T> T* get(const String& key) const` — retrieve the stored pointer cast to `T*`; returns `nullptr` if missing.

Usage pattern
-------------
1. During startup, register services:

```cpp
dependencyContainer.set<ConfigManager>("config", &config);
dependencyContainer.set<WiFiManager>("wifi", &wifiManager);
dependencyContainer.set<Face>("face", &face);
```

2. When mounting a `Router` or registering a `Command`, call `attachDependencies(&dependencyContainer)` so the caller can use `use<T>("key")` inside handlers.

Safety notes and pitfalls
------------------------
- Raw pointers: the container stores raw pointers. It's the caller's responsibility to ensure objects outlive the container entries. Do not register pointers to stack-allocated objects unless their lifetime is guaranteed.
- Type safety: the `get<T>()` template performs a C-style cast. Mismatched types can lead to undefined behavior. Use consistent keys and document the expected type for each key.
- Null checks: always check the result of `get<T>()` for `nullptr` before dereferencing.

Threading and concurrency
-------------------------
- The implementation is not synchronized. If services are registered from one task and read from another concurrently, add a mutex around `set`/`get` operations.

Testing strategies
------------------
- Unit tests can use a lightweight container instance and register mock objects (or lambdas wrapped in small structs), then call handlers that expect those dependencies.

Alternatives and improvements
-----------------------------
- Replace with a typed DI container (e.g., templated map or a small registry) to avoid unsafe casts.
- Add lifecycle management (construct/destroy) so the container can own services and clean them up at shutdown.

Cross-references
----------------
- See `docs/firmware/Command.md` and `docs/firmware/Router.md` to learn how `use<T>()` is used in handlers.
