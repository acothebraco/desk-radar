# Data source — ADS-B feed

## Primary: airplanes.live (access may require contributor approval)
Independent community-owned ADS-B/MLAT aggregator. Requests may be refused (HTTP 403) when not authorized. DeskRadar automatically backs off and tries the alternatives. Respect the operator's access and rate-limit policies.

### Endpoint (position + radius)
```
GET https://api.airplanes.live/v2/point/{lat}/{lon}/{radius_nm}
```
- `lat`, `lon`: decimal degrees (our home coords).
- `radius_nm`: nautical miles (max 250). Convert from our range: `nm = km * 0.539957`.

### Response (readsb format)
JSON object; the aircraft list is under key **`ac`** (older/raw readsb files use `aircraft` — handle both). Each entry includes (keys omitted when unavailable):

| Field          | Meaning                                  | Use |
|----------------|------------------------------------------|-----|
| `hex`          | 24-bit ICAO id (may start with `~`)      | stable key / de-dupe |
| `flight`       | callsign (8 chars)                       | label |
| `lat`,`lon`    | position, decimal degrees                | project to screen |
| `alt_baro`     | barometric altitude (ft) or `"ground"`   | altitude color |
| `track`        | ground track, ° from true N             | glyph rotation |
| `true_heading` | heading, ° (fallback when no `track`)   | glyph rotation |
| `gs`           | ground speed (kt)                        | detail card |
| `baro_rate`    | vertical rate (fpm, ±)                   | V/S arrow |
| `squawk`       | transponder code                         | emergency detect (7500/7600/7700) |
| `seen_pos`     | seconds since last position fix          | stale/expiry + trail |
| `t` / `type`   | aircraft type (e.g. B738) when present   | detail card |
| `dbFlags`      | bitfield (military, etc.)                | "interesting" alerts |

### Secondary: adsb.fi

Public endpoint: `https://opendata.adsb.fi/api/v3/lat/{lat}/lon/{lon}/dist/{radius_nm}`.
Credit: [adsb.fi](https://adsb.fi/). **Attribution and a link to adsb.fi are required** by the provider; public API use is personal and non-commercial, with a documented 1 request/second limit.

## Fallback: adsb.lol

Endpoint: `https://api.adsb.lol/v2/point/{lat}/{lon}/{radius_nm}`.
Credit: [adsb.lol](https://adsb.lol/). Public request limits can vary; DeskRadar adjusts its request rate after HTTP 429.

## Provider control

- HTTP 403: temporarily park provider with escalating cooldowns rather than retrying continuously.
- HTTP 429: increase per-provider request spacing and honor `Retry-After`.
- Recover to normal request spacing after successful polls.
- HTTPS uses limited connection and handshake timeouts; HTTP/1.0 avoids raw chunked JSON decoding problems.
- User-Agent identifies DeskRadar and its current firmware version.

## On-device math (implemented in src/geo.h)
For each aircraft, given home `(lat0, lon0)` and range `R_km` (outer ring):
1. **Distance** via haversine (km).
2. **Bearing** from home to aircraft (° from N, clockwise).
3. **Project** to screen: `r_px = (dist_km / R_km) * R_px_outer`; with north-up,
   `x = cx + r_px * sin(bearing)`, `y = cy - r_px * cos(bearing)`.
4. Drop aircraft beyond `R_km` (or clamp to the rim with a "beyond range" marker).
