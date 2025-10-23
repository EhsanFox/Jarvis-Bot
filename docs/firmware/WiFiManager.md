WiFiManager (lib/WiFiManager)
================================

Purpose
-------
`WiFiManager` wraps ESP32 `WiFi` APIs and provides helpers for station (STA) and access point (AP) modes, scanning and convenience flows used by the web UI and CLI.

API summary
-----------
- `WiFiManager(const String& ssid = "", const String& password = "", const String& apName = "ESP32-AP", const String& apPassword = "")`
- `void begin()` — prepare Wi-Fi subsystem (sets `WIFI_STA` by default).
- `bool connectSTA(unsigned long timeout = 10000)` — connect using stored `_ssid` and `_password`.
- `bool connectTo(const String& ssid, const String& password = "", unsigned long timeout = 10000)` — connect to provided credentials.
- `String tryConnect(const String& ssid, const String& password = "", unsigned long timeout = 10000)` — attempts connection while leaving AP interface available; returns IP string on success or empty string on failure.
- `void startAP()` / `void stopAP()` — manage AP mode.
- `void startScan()` / `bool isScanComplete()` / `DynamicJsonDocument getScanResults()` — asynchronous scan support and JSON-serializable results.
- `bool isConnected() const; String ipAddress() const; void disconnect();`

Behavior and integration
-------------------------
This class aims to make common Wi‑Fi tasks easier and safer for a device that must support both AP-based setup and normal STA operation.

Key behavioral notes
--------------------
- Mode transitions: `tryConnect()` switches the device to `WIFI_AP_STA` so the device keeps an AP active during connection attempts. On success, it returns the station IP (as string) and the AP is stopped later by a background task.
- Non-blocking scan: `startScan()` calls `WiFi.scanNetworks(true)` for an asynchronous scan. Check `isScanComplete()` to know when `getScanResults()` is safe.
- `getScanResults()` returns a `DynamicJsonDocument` with an array of networks and calls `startScan()` again to keep results warm for the next request.

Detailed method semantics and edge behaviors
-------------------------------------------
- `begin()` — puts WiFi into `WIFI_STA` mode.
- `connectSTA(timeout)` — attempts a blocking connect using `_ssid`/`_password`. Returns `true` when connected before `timeout`. This is a blocking routine and should be used sparingly (e.g., during boot) to avoid blocking asynchronous tasks.
- `connectTo(ssid, password, timeout)` — similar to `connectSTA` but uses provided credentials.
- `tryConnect(ssid, password, timeout)` — the preferred method for web-based connect flows. It sets `WIFI_AP_STA`, starts the connection attempt and waits up to `timeout` for `WL_CONNECTED`. On success it prints and returns the local IP; on failure returns empty string. It also yields (`yield()`) during the wait loop to allow other background processes to run.
- `stopAP()` — cleansly stops the soft AP (if running) and returns the WiFi mode to `WIFI_STA` while trying to keep any existing STA connection alive.

Programming examples
--------------------
1) Boot-time connect (blocking):

```cpp
WiFiManager wifi(cfg.get("wifi.ssid").as<String>(), cfg.get("wifi.password").as<String>());
wifi.begin();
if(!wifi.connectSTA(10000)) {
	wifi.startAP();
}
```

2) Connect from web UI flow (non-blocking for AP):

```cpp
String ip = wifi.tryConnect(formSsid, formPassword, 4000);
if(ip != "") {
	 // persist credentials and schedule AP shutdown
}
```

Robustness and pitfalls
-----------------------
- Blocking calls: `connectSTA` blocks the caller; don't run it on performance-critical tasks. The web flows use `tryConnect()` that yields and checks status intermittently.
- WDT and long loops: ensure you `yield()` or `delay()` with small intervals in connection loops to avoid watchdog resets.
- AP vs STA coexistence: ESP32 supports `WIFI_AP_STA` but keep in mind that throughput may be reduced and behavior depends on underlying WiFi driver.

Testing & validation
--------------------
- Practice flows: test these scenarios on hardware
	1. Fresh device — start AP and complete web wizard.
	2. Wrong credentials — ensure `tryConnect` returns empty and AP remains active.
	3. Stop AP after connect — ensure `stopAP()` leaves STA active.

Recommended improvements
------------------------
- Make scanning results configurable (e.g., limit number of results, filter by RSSI).
- Add exponential backoff/retry policies for connection attempts.
- Provide callback hooks for connection success/failure events so higher layers can react without polling.
-------
`WiFiManager` wraps ESP32 `WiFi` APIs and provides helpers for station (STA) and access point (AP) modes, scanning and convenience flows used by the web UI and CLI.

API summary
-----------
- `WiFiManager(const String& ssid = "", const String& password = "", const String& apName = "ESP32-AP", const String& apPassword = "")`
- `void begin()` — prepare Wi-Fi subsystem (sets `WIFI_STA` by default).
- `bool connectSTA(unsigned long timeout = 10000)` — connect using stored `_ssid` and `_password`.
- `bool connectTo(const String& ssid, const String& password, unsigned long timeout = 10000)` — connect to provided credentials.
- `String tryConnect(const String& ssid, const String& password, unsigned long timeout = 10000)` — attempts connection while leaving AP interface available; returns IP string on success or empty string on failure.
- `void startAP()` / `void stopAP()` — manage AP mode.
- `void startScan()` / `bool isScanComplete()` / `DynamicJsonDocument getScanResults()` — asynchronous scan support and JSON-serializable results.
- `bool isConnected() const; String ipAddress() const; void disconnect();`

Behavior and integration
------------------------
- `tryConnect()` temporarily sets the device to `WIFI_AP_STA` to allow simultaneous AP and STA while attempting to connect; on success it returns the assigned IP.
- `getScanResults()` returns a `DynamicJsonDocument` of networks and immediately triggers another scan to keep results fresh for the UI.

Usage examples
--------------
1. From CLI: attempt to connect and persist credentials via `ConfigManager`.
2. From web UI: call `/wifi/list` then `/wifi/connect` and let the server create an access token and persist the new `hashedPassword`.

Notes & robustness
------------------
- The firmware takes care to not drop the STA interface unnecessarily when stopping the AP; `stopAP()` switches back to `WIFI_STA`.
- Scanning is asynchronous — check `isScanComplete()` before reading results.
