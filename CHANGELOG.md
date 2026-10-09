# Changelog

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
