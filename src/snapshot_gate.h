#pragma once
#include <stdint.h>

// Delay publishing a *valid* but empty ADS-B list; unsuccessful network polls
// never reach this gate. Millis arithmetic intentionally handles rollover.
class AircraftSnapshotGate {
public:
    bool shouldPublish(bool hasAircraft, uint32_t nowMs, uint32_t emptyGraceMs) {
        if (hasAircraft) {
            _emptyPending = false;
            return true;
        }
        if (!_emptyPending) {
            _emptyPending = true;
            _emptySinceMs = nowMs;
        }
        return (uint32_t)(nowMs - _emptySinceMs) >= emptyGraceMs;
    }
private:
    bool _emptyPending = false;
    uint32_t _emptySinceMs = 0;
};
