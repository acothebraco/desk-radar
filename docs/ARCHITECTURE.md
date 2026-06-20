# Architecture

## Concurrency model
- **`adsb_task` (pinned to core 0)**: maintains WiFi, fetches the feed every `POLL_INTERVAL_MS`, parses it, and atomically swaps the result into a shared `std::vector<Aircraft>` protected by `g_ac_mutex`. Handles failover (airplanes.live → adsb.lol), backoff on error, and marks/expires stale entries by `seen_pos`.
- **Render loop (core 1 / Arduino `loop()`)**: drives `lv_timer_handler()`, copies the aircraft snapshot under the mutex, and renders the active view. No blocking calls here.

```
[airplanes.live] --HTTPS--> adsb_task (core0) --mutex--> g_aircraft[] <--mutex-- render (core1) --> LVGL/Arduino_GFX --> AMOLED
                                                                                  ^ touch (CST9217) -> hit test
```

## Memory
- Full RGB565 framebuffer = 466×466×2 ≈ **434 KB** → PSRAM. Double-buffer fits easily in 8 MB.
- Allocate LVGL draw buffers in PSRAM (`heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`); keep partial-render buffers if full-frame proves heavy.

## Rendering the scope
Two viable approaches:
- **LVGL canvas / custom draw widget**: draw rings, sweep, glyphs each frame into an `lv_canvas`. Simplest integration with touch + the other LVGL screens.
- **Arduino_GFX direct draw** for the scope layer, LVGL for chrome (cards, lists). More control over the sweep gradient.
Start with the LVGL canvas; optimize only if frame time suffers. Glyphs are small polygons rotated by `track` (see mockup paths). Trails = short fading polyline from recent positions.

## Input
- Touch → screen coordinates → nearest-glyph hit test (within a tap radius) → select + populate detail card.
- Swipe gestures switch views; long-press cycles range.
- IMU posture from QMI8658 drives sleep/wake; LVGL indev for touch.

## Persistence & lifecycle
- `Preferences` (NVS): `wifi_ssid/pass`, `home_lat/lon`, `range_km`, `units`, `north_up`, `mute`.
- First boot or held BOOT → WiFiManager captive portal.
- ArduinoOTA enabled after WiFi is up.

## Files
- `geo.h` — pure math (complete): haversine, bearing, project-to-screen.
- `aircraft.h` — `Aircraft` struct + altitude→color + squawk/emergency helpers.
- `adsb_client.h/.cpp` — HTTPS GET + ArduinoJson parse into `std::vector<Aircraft>` (working draft; test on hardware).
- `radar_view.h` — scope render API (implement against LVGL).
- `main.cpp` — config, task creation, mutex, loop glue (skeleton).
