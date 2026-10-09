# DeskRadar v1.5.4 — validation before publication

This is a **release-candidate source package**, not a published release. Keep the working v1.5.3 firmware as a recovery option until the checks below pass.

## Additional weather functionality (this updated candidate)

- Fourth screen: swipe Radar → List → Stats → Weather; tap the bottom control to switch WX Radar → Sat Clouds → 3-Day Weather.
- Open-Meteo JSON forecast, RainViewer precipitation PNG and EUMETSAT WMS satellite JPEG.
- Staggered network fetches (forecast 30 min / precipitation 5 min / clouds 10 min); cached results survive short outages.
- PSRAM-owned double buffers for each image, plus a separate LVGL canvas buffer, with atomic buffer handover.
- Added `bitbank2/PNGdec` and `-DPNG_MAX_BUFFERED_PIXELS=8192` following upstream's 512px PNG decoder memory safety correction.
- **Important:** The earlier v1.5.4 hardware tests do not cover the weather add-on; rebuild and retest this ZIP.

## Changes in this package

- `src/main.cpp`: PSRAM-preferred mbedTLS allocator registered at boot, valid-empty snapshot grace, saved upper-altitude settings, new web feed status endpoint and UI.
- `src/adsb_client.cpp/.h`: upper-altitude filter alongside existing minimum/military filters.
- `src/radar_view.cpp`: optimized transparent alpha canvas reset.
- `src/snapshot_gate.h`, `src/altitude_filter.h`: small testable policy helpers.
- `src/config.h`: version 1.5.4.
- `.github/workflows/release.yml`: v1.5.4 release notes + host tests.
- `.github/workflows/webflasher.yml`: only builds/deploys from `main`, and stamps the web-flasher from the exact `FW_VERSION` compiled instead of a stale git tag.
- `README.md`, `CHANGELOG.md`, `RELEASE_NOTES_v1.5.4.md`: documentation.

## Checks completed in preparation

1. C++ host tests: snapshot grace period, recovery after a subsequent good snapshot, millis() rollover and altitude filter limits — PASS.
2. Web UI: number/type of printf format arguments and handlers/NVS references — PASS.
3. Embedded page JavaScript syntax (`node --check`) — PASS.
4. GitHub workflow configuration: source version and release note filenames — PASS.
5. Firmware file baseline verified against current GitHub main by Git blob SHA (all firmware files used matched).

**Not yet run for the weather candidate:** PlatformIO ESP32-S3 firmware build and on-device smoke test. The local authoring environment has no ESP32-S3 PlatformIO toolchain or device. GitHub is not updated and no v1.5.4 tag has been created.

## Required tests in VS Code (before Git publication)

1. Open `desk-radar` and select the `esp32-s3-amoled-175` environment in PlatformIO.
2. Run **Clean**, **Build**, then **Upload** over USB. Do **not** update via auto-OTA while verifying.
3. Boot screen disappears and radar view starts; 60+ seconds of sweep without freezing.
4. Aircraft continue arriving from adsb.fi/adsb.lol when airplanes.live refuses service.
5. Web portal `http://deskradar.local/` displays **v1.5.4** and an **ADS-B data feed** card.
6. Provider name and `Last successful fetch` update automatically; `http://deskradar.local/feedstatus` responds with JSON.
7. Set maximum altitude to `Below 10,000 ft`, verify it persists over reboot and is effective; restore filter to **Off**.
8. Brief WiFi outage restores automatically; recovery AP at `http://192.168.4.1/` remains usable during a longer outage.
9. Swipe into the weather view: verify all 3 weather modes and their source labels, including forecast (today + 3 days), RainViewer precipitation and EUMETSAT satellite clouds. Wait for each source to load over WiFi (up to ~30 seconds initially). If a provider blocks access, observe a graceful waiting/failure rather than a freeze.
10. Leave the weather tile visible for 10 minutes and return to Radar; confirm updates do not interrupt touch or the sweep. Confirm the saved/GPS centre determines the image. Disconnect WiFi temporarily and verify previously received data remains visible.
11. Standby on/off, display brightness, all four themes, touch, trails, map, airport markers, alerts and OTA web page continue to operate.
12. Leave running for 1–2 hours, watching for crashes, abnormal memory loss or missing data.

Only after that pass, synchronize the source to your Git clone, build via GitHub Actions and then tag `v1.5.4`.

## After hardware verification: commands in VS Code

From the folder containing the ZIP's extracted `desk-radar` and a **separate existing Git clone** at `..\\desk-radar-github`:

```powershell
# Your terminal must be in the extracted v1.5.4 desk-radar directory.
# The following copy keeps .git and local VS Code/PlatformIO build files out.
robocopy . ..\\desk-radar-github /E /XD .git .pio .vscode /XF *.bin *.elf *.map
cd ..\\desk-radar-github
git status
git add -A
git commit -m "DeskRadar v1.5.4 - weather radar, satellite clouds, 3-day forecast and stability"
git push origin main
```

`robocopy` exits with codes 0 through 7 on success or nonfatal differences; if using a script, regard only codes 8 or higher as errors. Confirm the Git clone has no unrelated changes first.

Once the `main` workflow is green and its web-flasher is working, tag the release:

```powershell
git tag -a v1.5.4 -m "DeskRadar v1.5.4"
git push origin v1.5.4
```

The release workflow builds `DeskRadar-esp32s3.bin`, `DeskRadar-ota.bin` and uses `RELEASE_NOTES_v1.5.4.md` as the release description. The web flasher is deployed from `main`; release tags do not deploy Pages.
