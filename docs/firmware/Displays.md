Displays (include/Displays.h)
=================================

Purpose
-------
`Displays.h` centralizes hardware constants and default display geometry used by the `Face` rendering subsystem and the `u8g2` display drivers. Keeping display parameters in one header simplifies hardware changes and reduces magic numbers across the codebase.

Public constants
----------------
- `SDA_PIN` — default I2C SDA pin (board-specific; default in this project: 21).
- `SCL_PIN` — default I2C SCL pin (default: 22).
- `SCREEN_WIDTH`, `SCREEN_HEIGHT` — framebuffer size used to configure `U8G2` and `Face` drawing primitives (defaults: 128x64).

Hardware and driver notes
------------------------
- I2C addresses: the project uses two SSD1306 displays with different I2C addresses (commonly 0x3C and 0x3D). Those addresses are configured where `U8G2` objects are instantiated in `FaceManager`.
- Full-frame buffering: `U8G2` drivers use a full framebuffer allocation. On an ESP32, this can consume a noticeable portion of heap—watch memory usage when enabling both displays and large animations.

Changing display geometry
-------------------------
If you switch to a different display size or driver:
1. Update `SCREEN_WIDTH` and `SCREEN_HEIGHT`.
2. Verify all `EyeDrawer` and `Face` code that assumes center coordinates or hardcoded offsets still operate correctly.
3. Re-run visual regression checks (if available) to validate alignment.

Best practices
--------------
- Keep hardware-specific pin constants in `Displays.h` and do not duplicate them elsewhere.
- When testing on different boards, provide a small wrapper or `#ifdef` regions to select pin values per board.

Troubleshooting
---------------
- Symptoms: blank display, flicker or garbled characters can come from incorrect I2C pins, wrong address, or insufficient heap for full buffers.
- Tools: use `Wire.begin(SDA_PIN, SCL_PIN)` in a minimal sketch to confirm the displays respond to scans (`Wire.requestFrom` / `i2cscanner` sketch).

Cross-references
----------------
- See `docs/firmware/lib/FaceManager.md` for how display geometry is used by the face rendering pipeline and where `u8g2` instances are created.
