# Setup

## Toolchain
- VS Code + **PlatformIO** (recommended for Claude Code) OR Arduino IDE 2.x.
- USB-C cable. On first flash you may need to hold **BOOT** then tap **PWR/RST**.

## PlatformIO
```
pio run                      # build
pio run -t upload            # flash
pio device monitor -b 115200
```

## Bring-up order (important)
1. Clone the Waveshare factory Arduino demos from the wiki:
   https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.75
2. Build & flash `01_HelloWorld`. Confirm the screen lights up.
3. **Copy the exact pins** (QSPI SCLK/D0..D3, I2C SDA/SCL, ES8311 I2S) from those demos into `src/config.h` (replace the `-1` placeholders).
4. Build `06_LVGL_Widgets` and copy its working **`lv_conf.h`** into this project's include path (we set `-DLV_CONF_INCLUDE_SIMPLE`). Match the LVGL major version in `platformio.ini` to that demo.
5. Now build this project and proceed through the milestones in `CLAUDE.md`.

## lv_conf.h
LVGL needs `lv_conf.h` reachable on the include path. Easiest: copy from the Waveshare `06_LVGL_Widgets` demo (it's already tuned for this panel/color depth), set `LV_COLOR_DEPTH 16`, enable PSRAM draw buffers, and keep the QMI8658/touch indev wiring from demos `03/04`.

## WiFi & location
No secrets are committed. On first boot the captive portal collects SSID/password and home lat/lon. Defaults in `src/config.h` are Dénia (38.8409, 0.1059) — change as needed.

## HTTPS note
airplanes.live is HTTPS. For a hobby build, `WiFiClientSecure::setInsecure()` is fine. For production, pin the root CA. The choice is flagged in `adsb_client.cpp`.
