FaceManager (lib/FaceManager)
===================================

Comprehensive overview
----------------------
The `FaceManager` library implements the expressive face subsystem used by the device. It is intentionally modular so the rendering, temporal behavior and high-level expressions are separate concepts:

- Low-level rendering: `EyeDrawer` converts an `EyeConfig` (geometry/shape) into drawing operations against `U8G2`.
- Spatial transforms: `EyeTransformation`, `EyeTransition` allow interpolation between shapes and transformations (scale/move) using `Animations` primitives.
- Temporal variation: `EyeVariation` and `EyeBlink` add periodic or event-driven changes.
- High-level orchestration: `Face`, `FaceBehavior`, `FaceExpression`, `BlinkAssistant` and `LookAssistant` compose lower layers into natural motion.

Key classes and responsibilities
--------------------------------
- `Face` — top-level API used by the rest of the firmware. Responsibilities:
	- Initialize both displays (`u8g2_left` and `u8g2_right`).
	- Hold left/right `Eye` objects and the `Blink`/`Look` assistants.
	- Expose `Update()`, `DoBlink()` and `Look*()` helpers.
	- Manage high-level flags: `RandomBehavior`, `RandomLook`, `RandomBlink`.

- `Eye` — encapsulates a pipeline of operators that map a base `EyeConfig` into the final drawable `EyeConfig`:
	- `Transition` (smoothly transitions between configs)
	- `Transformation` (move/scale)
	- `Variation1/2` (small periodic offsets)
	- `BlinkTransformation` (applies blink geometry)
	- `FinalConfig` pointer used by `EyeDrawer`

- `EyeDrawer` — performs rasterization into the `U8G2` buffer. The drawer includes careful handling of rounded corners, slopes and anti‑overrun checks (e.g., radius vs height).

- `FaceExpression` — provides a library of emotion presets (e.g., `GoTo_Happy()`, `GoTo_Angry()`), implemented by applying presets in `EyePresets.h` and tuning variations.

- `FaceBehavior` — roulette-based emotional selector that randomly changes expressions based on weights.

- `BlinkAssistant` and `LookAssistant` — time-based helpers that perform periodic blinks and random look moves using `AsyncTimer`.

Public API (practical developer view)
------------------------------------
- Construction: `Face(uint16_t screenWidth, uint16_t screenHeight, uint16_t eyeSize)` — constructs and initializes both SSD1306 displays and all internal assistants.
- `void Update()` — call every main loop iteration to run assistants and draw frames.
- `void DoBlink()` — force an immediate blink.
- Look helpers: `LookLeft()`, `LookRight()`, `LookFront()`, `LookTop()`, `LookBottom()` — convenience wrappers.
- Access internals: `face.LeftEye`, `face.RightEye`, `face.Expression`, `face.Behavior` for fine-grained control.

Rendering pipeline and timing
----------------------------
Each frame, `Face::Update()` runs the following steps:

1. Update behavior assistant (maybe change emotion)
2. Update look assistant (maybe change transforms)
3. Update blink assistant (maybe trigger blink)
4. Call `Draw()` which:
	 - Clears buffers for left and right displays
	 - Updates `Eye` internal chain (`Transition.Update()`, `Transformation.Update()`, ...)
	 - Calls `EyeDrawer::Draw()` for each eye into the `U8G2` buffer
	 - Calls `u8g2.sendBuffer()` to write the buffer to the hardware

Because `U8G2` uses a frame buffer (the `_F_` variant), `sendBuffer()` can be expensive on slow I2C links; keep per-frame complexity low and avoid large draw ops every loop if not necessary.

Presets and geometry
---------------------
- `EyePresets.h` contains many hand-tuned `EyeConfig` presets (Normal, Happy, Sad, Surprised, etc.). Each preset sets Offset, Width/Height, Slope and corner radii.
- `EyeConfig` fields (OffsetX/OffsetY, Height, Width, Slope_Top, Slope_Bottom, Radius_Top/Bottom, etc.) provide complete control over the eye shape.

Practical examples
------------------
1) Initialize a face and set a mood:

```cpp
Face face(128, 64, 40);
face.Expression.GoTo_Happy();
// in loop()
face.Update();
```

2) Force a quick look and blink sequence:

```cpp
face.LookLeft();
face.DoBlink();
```

Performance considerations
--------------------------
- Buffering: current code uses `u8g2_left`/`u8g2_right` with `U8G2_SSD1306_128X64_NONAME_F_HW_I2C` — the `_F_` variant builds a full frame in RAM. Ensure your MCU has enough heap for two frame buffers plus runtime objects.
- I2C throughput: `sendBuffer()` can take tens to hundreds of milliseconds over I2C depending on clock rate and bus congestion. Avoid drawing at full speed if not necessary; consider throttling update frequency.
- Dynamic allocations: `FaceManager` itself avoids heavy dynamic allocations in the draw path, but your overall heap must accommodate `U8G2` buffers and small `Animation` objects.

Debugging and visual testing
---------------------------
- Serial logging: add Serial prints in `Face::Draw()` or `Eye::ApplyPreset()` to validate transitions and values.
- Visual snapshots: capture display frames with a camera or use a display simulator to verify expressions.

Extensibility
-------------
- Add new expressions: implement `GoTo_NewExpression()` in `FaceExpression.cpp` that sets transitions and variation values.
- Add new animations: extend `Animations.h` with additional curves or timing modes.

Edge cases and robustness
-------------------------
- Radius vs height: `EyeDrawer::Draw()` adjusts radii if requested radii would exceed eye height; this avoids drawing artifacts but means presets must be tested at multiple sizes.
- Mirroring: `Eye::ApplyPreset()` flips offsets and slopes for mirrored eyes. If adding asymmetrical eyes, watch for sign conventions.

Testing suggestions
-------------------
- Unit test: for transformation math, write small tests to validate `EyeTransformation::Apply()` and `EyeTransition::Apply()` produce numerically stable results.
- Visual regression: maintain a set of presets and render frames to a test harness to capture images and compare pixel diffs.

Recommended improvements
------------------------
- Add a configuration-driven framerate limiter to avoid excessive I2C writes.
- Expose more fine-grained control through `ConfigManager` for tuning (blink interval, variation amplitudes, look timings).
