ConfigManager (lib/ConfigManager)
===================================

Purpose
-------
`ConfigManager` is a singleton that manages persistent configuration stored on LittleFS (default `/config.json`). It's used for Wi-Fi credentials, JWT secret, face settings and other persisted state.

API
---
- `ConfigManager(const char* filePath = "/config.json")` — constructor. A singleton `getInstance()` is available in the header.
- `JsonVariant get(const String& key)` — dot-separated key access, returns a `JsonVariant` which can be cast using `.as<T>()` or `as<String>()`.
- `bool set(const String& key, const JsonVariant& value)` — persist a value and save file.
- Convenience overloads: `set(key, String)`, `set(key, bool)`, `set(key, int)`.

Behavior
--------
- On construction, `ConfigManager` mounts LittleFS and attempts to load the config file. If it doesn't exist, it initializes an empty document and saves a new file.
- `get()` supports dot-separated key paths (e.g., `face.width`). If a key is missing the `JsonVariant` will be empty.

Example
-------
ConfigManager& cfg = ConfigManager::getInstance();
String ssid = cfg.get("wifi.ssid").as<String>();
ConfigManager (lib/ConfigManager)
===================================

Purpose
-------
`ConfigManager` is the single source of truth for persistent configuration stored on LittleFS. The firmware uses it for:

- Wi‑Fi credentials and AP configuration
- JWT secret and hashed admin password
- Face rendering settings (size, width, height)
- Application flags like `isReady`

Design contract
---------------
- Persistence: configuration is stored as a JSON document on LittleFS (default path `/config.json`).
- API: dot-separated key lookup (`get("wifi.ssid")`) and typed `set` helpers. Callers should treat `ConfigManager` as the authoritative persisted store.
- Lifetime: `ConfigManager` is typically used as a singleton (`ConfigManager::getInstance()`) but the class supports normal construction with a file path if testing or multiple instances are required.

Public API (summary)
--------------------
- `ConfigManager(const char* filePath = "/config.json")` — constructor. Mounts LittleFS (if not already) and loads the file. If the file is missing it creates an empty document and writes a file.
- `static ConfigManager& getInstance()` — convenience singleton.
- `JsonVariant get(const String& key)` — returns a `JsonVariant` for a dot-separated key. Example: `cfg.get("wifi.ssid").as<String>()`.
- `bool set(const String& key, const JsonVariant& value)` — set a JSON value and persist to disk.
- Overloads: `set(key, const String&)`, `set(key, const bool&)`, `set(key, const int&)`.

Behavioral details and implementation notes
-----------------------------------------
- Loading: the constructor attempts to `LittleFS.begin(true)` (format if necessary) and then opens `_filePath` for read; a failed parse will clear `_doc` and return a load failure.
- Key lookup: `get()` traverses nested keys by splitting on `.` and walking the `DynamicJsonDocument`. If a segment is missing an empty `JsonVariant` is returned (check for `.isNull()` or cast safely).
- Atomicity: `set()` writes the entire `DynamicJsonDocument` to disk synchronously (open with "w"). Frequent writes will wear flash; batch updates where possible.
- Document sizing: `_doc` is created with 1024 bytes in the current implementation. If you store larger nested objects (e.g., UI assets or long lists), increase the `DynamicJsonDocument` capacity in `ConfigManager.h` and `ConfigManager.cpp`.

Threading and concurrency
-------------------------
- The ESP32 environment uses FreeRTOS. `ConfigManager` as implemented is not protected by mutexes; if you plan to call `set()` concurrently from different tasks, add a `std::mutex`/`portMUX_TYPE` lock around `load()`/`save()` and around `_doc` mutation.

Typical usage examples
----------------------
1) Read Wi‑Fi SSID and try connecting:

```cpp
ConfigManager& cfg = ConfigManager::getInstance();
String ssid = cfg.get("wifi.ssid").as<String>();
String pwd = cfg.get("wifi.password").as<String>();
if(!ssid.isEmpty()) wifi.connectTo(ssid, pwd);
```

2) Update nested object safely (example sets `isReady`):

```cpp
ConfigManager& cfg = ConfigManager::getInstance();
cfg.set("isReady", true);
```

3) Persist JSON object (e.g., full `wifi` object):

```cpp
DynamicJsonDocument d(256);
JsonObject w = d.to<JsonObject>();
w["ssid"] = "MyNet";
w["password"] = "secret";
cfg.set("wifi", d.as<JsonVariant>());
```

Edge cases, failure modes and troubleshooting
--------------------------------------------
- LittleFS mount failure: check Serial logs on boot; the constructor prints a message if mounting fails.
- Corrupted file: `deserializeJson` returns an error and `ConfigManager` clears the document. You may lose persisted settings; keeping a backup or migration strategy (copy to `/config.bak`) is recommended.
- Document too small: `deserializeJson` may fail due to insufficient capacity — increase `_doc` size.

Testing suggestions
-------------------
- Unit test: create a temporary LittleFS-backed environment and exercise `set/get` for nested keys and object persistence across reboots.
- Integration test: write a test that runs on-device which sets several keys, reboots the device and asserts that values persisted.

Performance & flash wear considerations
-------------------------------------
- Minimize number of writes. Combine multiple `set()` calls into one where possible.
- Consider journaling or write coalescing if you anticipate very frequent updates (e.g., sensor logs). For config it's usually acceptable to write rarely.

Recommended improvements
------------------------
- Add mutex protection for multi-task safety.
- Add a `saveDeferred()` that writes after a small debounce window to reduce flash wear.
- Add schema validation and migration helpers to safely evolve config structure.
