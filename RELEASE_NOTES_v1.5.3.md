# DeskRadar v1.5.3 — Reliable ADS-B Feed & WiFi Recovery

This release improves connectivity and stability for DeskRadar on the Waveshare ESP32-S3-Touch-AMOLED-1.75.

## What's new

- **Three ADS-B data sources:** automatically tries **airplanes.live**, **adsb.fi** and **adsb.lol** (with the correct API URL for each provider). If the primary API returns HTTP 403, DeskRadar continues with another available source.
- **Smarter API error handling:** per-provider cooldown after HTTP 403, adaptive request spacing after HTTP 429, `Retry-After` support, controlled HTTPS timeouts and more reliable JSON stream parsing.
- **Reliable WiFi recovery:** reconnect retries run without intentionally stopping the recovery AP. If WiFi remains unavailable for roughly 60 seconds, the `deskradar-Recovery` access point offers a local recovery page at **http://192.168.4.1/**. The device can reconnect automatically when WiFi returns.
- **Display startup and animation fix:** LVGL's tick is advanced consistently, so the boot logo exits and the radar sweep, aircraft and interface render normally.
- **LVGL font build fix:** enables required Montserrat font sizes for firmware builds.
- **Version display:** Web UI, radar UI, update checker and ADS-B User-Agent now identify the firmware as **v1.5.3**.
- **Improved serial diagnostics:** logs the active ADS-B provider and periodic UI state counters.

## Tested on hardware

The maintainer confirmed that radar sweep and aircraft rendering work, `opendata.adsb.fi` returns aircraft when `airplanes.live` rejects requests, and WiFi recovery works on the ESP32-S3-Touch-AMOLED-1.75. Source package/release workflow should still be rebuilt for v1.5.3 before publishing binaries.

## Firmware files

- **USB first-time flash:** `DeskRadar-esp32s3.bin`, merged image at offset `0x0`.
- **OTA update:** `DeskRadar-ota.bin` only. Upload at `http://deskradar.local/update` (do **not** upload the merged USB image).
- **Browser flash:** https://acothebraco.github.io/desk-radar/

## Data source credit and usage

Aircraft data is supplied by community data providers [airplanes.live](https://airplanes.live/), [adsb.fi](https://adsb.fi/) and [adsb.lol](https://adsb.lol/). **adsb.fi attribution is required** and its public service is for personal, non-commercial use. Access, rate limits and availability depend on each provider's policies. In particular, airplanes.live may return HTTP 403 to non-contributing clients; the firmware handles this by temporarily parking the provider rather than continuously retrying.
