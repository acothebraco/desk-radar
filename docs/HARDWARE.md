# Hardware — Waveshare ESP32-S3-Touch-AMOLED-1.75

## Board
- **MCU**: ESP32-S3R8 (Xtensa LX7 dual-core @240 MHz), 512 KB SRAM, **8 MB PSRAM**, **16 MB flash**.
- **Wireless**: 2.4 GHz WiFi (b/g/n) + Bluetooth 5 (LE), onboard antenna (IPEX option).
- **Display**: 1.75" AMOLED, **466×466**, 16.7M colors, driver **CO5300** over **QSPI**. Brightness set via panel command (no PWM backlight line).
- **Touch**: **CST9217** capacitive, **I2C**. (Some units carry FT3168 — detect/abstract.)
- **IMU**: **QMI8658** 6-axis (accel + gyro), I2C.
- **RTC**: **PCF85063**, I2C (addr 0x51), kept alive by the PMIC.
- **PMIC**: **AXP2101**, I2C (addr 0x34), LiPo charge + rails.
- **Audio**: **ES8311** codec + onboard speaker (MX1.25 header); dual-mic array (ES7210 on the -C variant) with echo cancellation.
- **Storage**: microSD (TF) slot.
- **Buttons**: PWR + BOOT (programmable).
- **Expansion**: 8-pin header = 3×GPIO + 1×UART, plus reserved I2C/IO pads.
- **GPS** (only on the `-G` variant): LC76G + external ceramic antenna.

## Pin map
### Verified (from the board's ESPHome definition)
| Signal            | GPIO | Notes                         |
|-------------------|------|-------------------------------|
| LCD CS            | 12   | CO5300, QSPI                  |
| LCD RST           | 39   | CO5300                        |
| Touch INT         | 11   | CST9217                       |
| Touch RST         | 40   | CST9217                       |
| Touch transform   | —    | mirror_x = true, mirror_y = true |

### To confirm from the official Waveshare Arduino demo (do NOT guess)
| Signal              | GPIO | Where to find it                 |
|---------------------|------|----------------------------------|
| LCD QSPI SCLK       | ?    | demo `01_HelloWorld` (Arduino_GFX_DataBus) |
| LCD QSPI D0..D3     | ?    | demo `01_HelloWorld`             |
| I2C SDA (shared)    | ?    | any LVGL+sensor demo (`03`/`04`) |
| I2C SCL (shared)    | ?    | any LVGL+sensor demo             |
| Speaker / I2S (ES8311) | ? | demo `08_ES8311`                 |

Touch, IMU, RTC, PMIC share one I2C bus. Pull pins from the demos into `src/config.h`; the placeholders there are `-1`.

## I2C addresses (typical)
- CST9217 touch: 0x15 (verify)
- QMI8658 IMU: 0x6B (or 0x6A)
- PCF85063 RTC: 0x51
- AXP2101 PMIC: 0x34

## Reference
Waveshare wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.75
ESPHome device page (pin source): https://devices.esphome.io/devices/waveshare-esp32-s3-touch-amoled-175/
