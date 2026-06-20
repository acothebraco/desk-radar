// LC76G GNSS over the shared I2C bus (Quectel I2C-NMEA protocol). Ported from Waveshare's
// ESP32-S3-LC76G-I2C example, but split into a non-blocking state machine so the two
// ~100 ms settle delays happen between loop() iterations instead of stalling the render.
#include "gps.h"
#include <Arduino.h>
#include <Wire.h>
#include <TinyGPS++.h>

#define GPS_ADDR_W   0x50    // write side (commands)
#define GPS_ADDR_R   0x54    // read side (length / data)
#define GPS_READ_MAX 120     // bytes per cycle (stay within the Wire buffer)
#define GPS_POLL_MS  1000    // how often to pull a chunk of NMEA
#define GPS_SETTLE_MS 110    // LC76G needs a moment between command and read

static const uint8_t QLEN[8] = { 0x08, 0x00, 0x51, 0xAA, 0x04, 0x00, 0x00, 0x00 };  // "how much is queued?"

static bool        s_present = false;
static TinyGPSPlus s_gps;

bool gps_begin() {
    Wire.beginTransmission(GPS_ADDR_W);
    Wire.write(QLEN, sizeof(QLEN));
    s_present = (Wire.endTransmission() == 0);   // 0x50 ACKs only on the -G variant
    Serial.printf("[gps] LC76G %s\n", s_present ? "detected on I2C" : "not present");
    return s_present;
}

bool gps_present() { return s_present; }

void gps_poll() {
    if (!s_present) return;
    enum { IDLE, WAIT_LEN, WAIT_DATA };
    static int      st = IDLE;
    static uint32_t t = 0;
    static uint32_t want = 0;
    const uint32_t now = millis();

    switch (st) {
        case IDLE:
            if (now - t < GPS_POLL_MS) return;
            Wire.beginTransmission(GPS_ADDR_W);
            Wire.write(QLEN, sizeof(QLEN));
            t = now;
            st = (Wire.endTransmission() == 0) ? WAIT_LEN : IDLE;
            return;

        case WAIT_LEN: {
            if (now - t < GPS_SETTLE_MS) return;
            uint32_t avail = 0;
            if (Wire.requestFrom(GPS_ADDR_R, 4) == 4) {
                const uint32_t b0 = Wire.read(), b1 = Wire.read(), b2 = Wire.read(), b3 = Wire.read();
                avail = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
            }
            if (avail == 0 || avail > 100000) { st = IDLE; t = now; return; }   // nothing / garbage
            want = (avail > GPS_READ_MAX) ? GPS_READ_MAX : avail;
            const uint8_t cmd[8] = { 0x00, 0x20, 0x51, 0xAA,
                                     (uint8_t)want, (uint8_t)(want >> 8),
                                     (uint8_t)(want >> 16), (uint8_t)(want >> 24) };
            Wire.beginTransmission(GPS_ADDR_W);
            Wire.write(cmd, sizeof(cmd));
            t = now;
            st = (Wire.endTransmission() == 0) ? WAIT_DATA : IDLE;
            return;
        }

        case WAIT_DATA: {
            if (now - t < GPS_SETTLE_MS) return;
            const int n = Wire.requestFrom(GPS_ADDR_R, (int)want);
            for (int i = 0; i < n; ++i) s_gps.encode((char)Wire.read());   // feed NMEA to the parser
            st = IDLE; t = now;
            return;
        }
    }
}

bool gps_has_fix() {
    return s_gps.location.isValid() && s_gps.location.age() < 10000;   // fix seen in the last 10 s
}

bool gps_location(double *lat, double *lon) {
    if (!gps_has_fix()) return false;
    if (lat) *lat = s_gps.location.lat();
    if (lon) *lon = s_gps.location.lng();
    return true;
}
