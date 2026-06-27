## DeskRadar v1.4.6

### New Feature: Automatic GitHub Firmware Updates

DeskRadar can now automatically check GitHub for new firmware releases.

#### Added

- Automatic firmware update check from GitHub releases
- New **Auto install firmware updates** checkbox on the configuration page
- New **Install update now** button on the configuration page
- DeskRadar checks GitHub periodically when auto update is enabled
- If a newer release is found, DeskRadar downloads DeskRadar-ota.bin
- Firmware is flashed automatically using OTA
- DeskRadar reboots after a successful update

#### Important

The automatic updater uses the OTA firmware file:

DeskRadar-ota.bin

It does **not** use the merged USB web flasher binary.

#### Recommended Use

Enable **Auto install firmware updates** on the configuration page if you want DeskRadar to keep itself updated automatically.
