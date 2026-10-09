# Changelog

## [1.5.4] - 2026-10-09

### Added
- Fourth touchscreen view for Weather: RainViewer precipitation radar, EUMETSAT satellite clouds and Open-Meteo three-day forecast.
- Scheduled, staggered downloads; cached PSRAM images and weather snapshots; waiting/attribution labels and on-screen mode switch.
- PNGdec decoding support and streamed HTTPS image retrieval with size validation.
- Maximum aircraft altitude filter (0 = disabled), adjustable live in the web interface and saved in NVS.
- Web dashboard ADS-B data source, last successful refresh age and feed status (auto-refreshed every 10 seconds).
- Unit-tested gate for transient valid-but-empty aircraft snapshots; retention up to `AC_STALE_MS`.

### Improved
- Use PSRAM-preferred mbedTLS record-buffer allocations with internal RAM fallback.
- Clear the flight-trail alpha canvas in one bulk `memset()` operation instead of a slow per-pixel LVGL fill.
- Preserve the actual last-fetch timestamp in standby mode.
- Add host tests to both release and web flasher workflows.

### Preserved
- v1.5.3 multi-provider ADS-B, WiFi recovery, standby, OTA, themes, radar UI, LVGL clock fix and font configuration.

## [1.5.3] - 2026-10-09

### Added
- Third aircraft feed `opendata.adsb.fi` with automatic provider failover.
- Per-provider HTTP 403 cooldown, HTTP 429 backoff and source diagnostics.
- Improved recovery WiFi access point availability after ~60 seconds offline.

### Fixed
- LVGL tick progression after boot; radar screen, sweep and animations run again.
- Missing Montserrat font definitions during ESP32 firmware linking.
- WiFi reconnection no longer deliberately interrupts an active recovery access point.
- Firmware version shown in web UI and on-device view now matches this release.

### Updated
- HTTPS timeouts, streaming JSON handling and ADS-B User-Agent.
- Web portal's recovery timing text and provider data attribution.

## [1.5.2] - 2026-07-17
- Previous release (see GitHub release history).
