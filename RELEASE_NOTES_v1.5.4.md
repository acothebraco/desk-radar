# DeskRadar v1.5.4 – Stability, Feed Diagnostics & Three Weather Views

DeskRadar v1.5.4 focuses on responsiveness and long-running reliability of the Waveshare ESP32-S3-Touch-AMOLED-1.75 without changing the familiar radar experience.

## Weather add-on (new in this updated v1.5.4 candidate)

Swipe **Radar → Aircraft → Stats → Weather**. Inside Weather, tap the bottom mode button (or the panel) to cycle through:

- **WX Radar** — precipitation echoes from RainViewer, centred on the saved or GPS location; normally updated every **5 minutes**. Cached 360×360 RGB565 radar image in PSRAM. If RainViewer is unavailable or no echo is present, the display retains its last valid image or shows a waiting message. Echoes indicate precipitation, not normal clouds.
- **Sat Clouds** — EUMETSAT Meteosat Cloud Type RGB via EUMETView WMS; approximately 400 km local satellite view, updated every **10 minutes**. Shows cloud classifications, not rainfall intensity.
- **3-Day Weather** — current temperature, apparent temperature, humidity and wind plus a three-day daily high/low and rain probability, powered by **Open-Meteo**; normally updated every **30 minutes**.

All three use the configured home/GPS centre, respect Aviation/Metric/Imperial unit display, stagger their initial downloads after WiFi connects, retry failed requests and keep their last valid data while offline. Image buffers are allocated in PSRAM, and the LVGL weather image is copied into its own stable front buffer on the display thread.

**Data acknowledgement:** [Open-Meteo](https://open-meteo.com/), [RainViewer](https://www.rainviewer.com/api.html) and [EUMETSAT EUMETView](https://view.eumetsat.int/). Data availability and personal/noncommercial access policies are determined by those providers.

## What's new

- **Less internal memory pressure:** mbedTLS allocates HTTPS buffers from PSRAM when available, with an internal-RAM fallback. Helps mitigate heap fragmentation across repeated ADS-B requests.
- **Smoother flight trails:** bulk-clear transparent radar flow canvas instead of clearing every pixel via LVGL.
- **Cleaner feed interruptions:** one valid-but-empty aircraft response no longer immediately wipes the radar; existing contacts remain for a short grace interval. Network errors continue to retain last good contacts.
- **Maximum altitude setting:** new saved filter in the configuration page (disabled, 3,000, 5,000, 10,000, 20,000, 33,000 or 45,000 ft). Combines with the existing minimum-altitude and military filters.
- **Live ADS-B diagnostics:** new web UI card shows the provider that last delivered data, elapsed time since last successful fetch, and whether the feed is fresh. New endpoint `GET /feedstatus` returns machine-readable JSON.
- **Version consistency:** `FW_VERSION` and embedded web UI show v1.5.4, and the release workflow publishes these notes.

## Unchanged

Four radar themes, ADS-B failover (airplanes.live / adsb.fi / adsb.lol), WiFi recovery AP, OTA, automatic update checks, standby mode, display animations and radar sweep are preserved.

## Installation

- **First flash over USB:** `DeskRadar-esp32s3.bin` (merged image at offset 0x0).
- **OTA update:** `DeskRadar-ota.bin` via `http://deskradar.local/update`.
- **Browser installer:** https://acothebraco.github.io/desk-radar/

## Credits & service limitations

ADS-B sources: [airplanes.live](https://airplanes.live/), [adsb.fi](https://adsb.fi/) and [adsb.lol](https://adsb.lol/). Follow their access requirements, rate limits and attribution rules; adsb.fi asks for linked attribution and permits personal/noncommercial usage.

## Testing and release status

Host tests cover empty-snapshot retention, millis rollover, altitude-filter boundaries, and weather-image buffer handover. The previous (non-weather) v1.5.4 candidate built and passed hardware testing. **The updated weather candidate must be compiled and flashed again before release.**
