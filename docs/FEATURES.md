# Features — Plane Radar 2.0

Visual target: `assets/plane_radar_2.0_mockup.html`.

## Look (the "prettier")
- True-black AMOLED background; phosphor-green scope, glow on the sweep edge.
- Concentric range rings + crosshair + N/E/S/W rose; outer-ring range label (e.g. "15 km").
- Rotating sweep with a trailing alpha gradient.
- Aircraft as **plane glyphs rotated by track/heading** (not dots).
- **Altitude color map**: ≤3k ft red → 3–10k amber → 10–20k lime → 20–30k green → 30k+ cyan.
- **Fading trail** behind each aircraft (last N positions / direction tail).
- Center "you" dot with a soft pulse.
- Top HUD: WiFi strength, aircraft count, clock.

## Functions (the "more")
1. **Touch to inspect** — tap nearest glyph → detail card: callsign, type, registration (if available), altitude, ground speed, vertical-rate arrow, distance, bearing, squawk.
2. **Views** (swipe): Radar · List (sorted by distance) · Detail · Stats (count, closest, max alt, msgs).
3. **Range zoom** — cycle 5 / 10 / 25 / 100 km (tap or long-press), re-query radius accordingly.
4. **Orientation** — north-up ↔ track-up toggle.
5. **Alerts** — highlight + speaker **ping** for: emergency squawks (7500/7600/7700), military (`dbFlags`), or a user watch-list of types (A380, B52…). Card flashes red (see RESCUE51 in the mockup).
6. **Night auto-dim** — use PCF85063 RTC to lower brightness after dusk.
7. **IMU gestures** (QMI8658) — face-down → screen sleep; face-up → wake; shake → force refresh.
8. **Setup & maintenance** — first-boot **captive portal** (WiFi creds + home lat/lon + range); settings in NVS; **OTA** updates.

## Nice-to-have / later
- Route enrichment (origin→destination) via a secondary lookup.
- Sound themes / mute.
- Multiple saved home locations.
- microSD logging of seen aircraft.

## MakerWorld packaging
- Parametric printable enclosure for the round board (bezel + stand), à la the original radar.
- Publish firmware + STLs; include a looping GIF of the live sweep. The original radar earned MakerWorld "featured/boost" badges — same formula here for points toward the P2S.
