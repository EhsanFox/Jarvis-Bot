prebuild.py
=================

Purpose
-------
`prebuild.py` is an extra PlatformIO script used during the build/upload lifecycle. In this project it prepares the project to ensure the device filesystem image (LittleFS) is built and available for upload. It's invoked by PlatformIO because it is referenced under `extra_scripts` in `platformio.ini`.

What it does (high level)
-------------------------
- Detects the PlatformIO build environment and registers hooks so the filesystem image (`data` -> LittleFS) gets built and uploaded at the appropriate time.
- Optionally runs preparatory steps before the main firmware upload (for example, ensuring `data/web` contains the front-end files). In this repo the heavy lifting is done by `postbuild.py` which builds and copies frontend assets; `prebuild.py` coordinates the PlatformIO hook chain.

When it runs
------------
- PlatformIO calls extra scripts at specific lifecycle points. `prebuild.py` is intended to run before or around the `upload` step. Its main role is to ensure any dependent artifacts are prepared so that `uploadfs` (the filesystem upload) has the latest content.

How it integrates with `platformio.ini`
--------------------------------------
Typical `platformio.ini` excerpt referencing this script:

```ini
extra_scripts = firmware/scripts/prebuild.py
```

Inside `prebuild.py` you will see code that attaches to the `env.AddPostAction` or similar PlatformIO hooks. That hook causes `pio run --target uploadfs` or a custom upload step to execute after `upload` or before it depending on hook placement.

Manual runs and debugging
-------------------------
- You can simulate the effect of the script by running PlatformIO targets manually. For example (PowerShell):

```powershell
# build firmware only
platformio run --environment esp32dev
# build and upload filesystem image to device
platformio run --environment esp32dev --target uploadfs
# upload firmware
platformio run --environment esp32dev --target upload
```

Troubleshooting
---------------
- If filesystem upload doesn't include expected files (e.g., frontend `index.html`), ensure `postbuild.py` has copied assets into `firmware/data/web` before `uploadfs` runs.
- Check PlatformIO verbose logs to see hook order:

```powershell
platformio run -v --environment esp32dev --target upload
```

- If the script fails with Python exceptions, run it directly with Python to get full traceback (PowerShell):

```powershell
python .\firmware\scripts\prebuild.py
```

Best practices and CI
---------------------
- In CI pipelines, make sure Node.js and npm are installed if `postbuild.py` expects to run the frontend build.
- Prefer deterministic builds: pin Node versions with an `.nvmrc` or use a fixed Docker image for CI.

Security considerations
-----------------------
- These scripts execute shell commands and arbitrary build tools. Avoid running untrusted modifications and ensure your CI runner has limited permissions.

Possible improvements
---------------------
- Add a dry-run flag or `--skip-frontend` environment variable to speed up quick firmware-only development cycles.
- Make the script emit structured logs (JSON) for CI parsing.
