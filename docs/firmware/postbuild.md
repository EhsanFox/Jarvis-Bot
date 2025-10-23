postbuild.py
=================

Purpose
-------
`postbuild.py` builds the frontend and prepares the filesystem image that will be uploaded to the device. It is invoked from the PlatformIO extra script hooks (commonly after `upload` or before `uploadfs`) and copies the static site into `firmware/data/web` so `uploadfs` can flash it to LittleFS.

Detailed behavior
-----------------
1. Locate the frontend project directory. By default the script expects a sibling `frontend/` folder next to `firmware/`.
2. Run the frontend build command (commonly `npm run build` or `pnpm build`).
3. Copy the build output (`dist`, `build`, or framework-specific output) into `firmware/data/web`.
4. Ensure `firmware/data/config.json` exists by copying `config.example.json` into `data/config.json` when missing—this guarantees the device has a baseline configuration file on first flash.
5. Optionally call `platformio run --target uploadfs` to upload the filesystem image if the script is configured to trigger the upload.

Example commands executed (conceptual)
-------------------------------------
- Change dir to `../frontend` and run `npm ci && npm run build` (or `npm install` depending on CI).
- Copy `frontend/dist/*` into `firmware/data/web/` (Windows command examples below).

PowerShell example (manual run)

```powershell
# from repo root
Set-Location .\frontend
npm ci
npm run build
Set-Location ..\firmware
Remove-Item -Recurse -Force .\data\web\*
Copy-Item -Recurse ..\frontend\dist\* .\data\web\
if(-not (Test-Path .\data\config.json)) { Copy-Item ..\config.example.json .\data\config.json }
# build filesystem image
platformio run --environment esp32dev --target buildfs
# upload filesystem image
platformio run --environment esp32dev --target uploadfs
```

Error handling and common failures
---------------------------------
- Node/npm not found: the script will fail when invoking `npm`. Install Node.js and ensure it's on PATH. In CI use official node images or install node before running the pipeline.
- Build failures: frontend frameworks will return non-zero exit status on failed builds—inspect logs in `frontend/.vite` or the framework's output.
- File copy problems: permission or path issues on Windows (long paths) can break copy. Use `robocopy` on Windows for robust directory syncing in CI.

CI recommendations
------------------
- Use `npm ci` in CI to ensure deterministic installs (requires package-lock.json).
- Cache `node_modules` between CI runs if using slow dependency installs; or use a build matrix that prebuilds the frontend artifact once and reuses it.
- If your CI runs on Linux but local dev is Windows, ensure file path handling in the script is cross-platform — prefer Python `shutil` for copying or use Node build scripts.

Security and reproducibility
---------------------------
- Do not commit built artifacts to the repo. Keep built files out of source control to avoid drift.
- Pin Node/tool versions via `.nvmrc` or lockfiles to get reproducible builds.

Possible improvements
---------------------
- Add a `--skip-frontend` flag to allow building only firmware.
- Support detecting the frontend toolchain (`vite`, `react-scripts`, `next`) and choose the correct build output folder automatically.
- Use `rsync`/`robocopy` for efficient and robust copying in CI.

Cross-references
----------------
- See `platformio.ini` for how `extra_scripts` references these scripts.
- See `docs/firmware/README.md` for an overview of the firmware build and upload flow.
