#include <cassert>
#include <cstdint>
#include <cstdio>
#include "snapshot_gate.h"
#include "altitude_filter.h"

int main() {
    AircraftSnapshotGate gate;
    // Real contacts always publish; a single empty payload must not erase them.
    assert(gate.shouldPublish(true, 1000, 15000));
    assert(!gate.shouldPublish(false, 2000, 15000));
    assert(!gate.shouldPublish(false, 16999, 15000));
    assert(gate.shouldPublish(false, 17000, 15000));
    assert(gate.shouldPublish(true, 17100, 15000));
    assert(!gate.shouldPublish(false, 18000, 15000)); // grace restarts
    // Rollover-safe elapsed-time comparisons.
    AircraftSnapshotGate wrap;
    assert(!wrap.shouldPublish(false, UINT32_MAX - 1000, 15000));
    assert(!wrap.shouldPublish(false, 13000, 15000));
    assert(wrap.shouldPublish(false, 14500, 15000));
    // Filters: disabled; lower boundary inclusive; upper boundary inclusive.
    assert(altitude_filter_accepts(false, 6000, 0, 0));
    assert(altitude_filter_accepts(false, 10000, 5000, 10000));
    assert(!altitude_filter_accepts(false, 10001, 5000, 10000));
    assert(!altitude_filter_accepts(false, 4999, 5000, 10000));
    assert(altitude_filter_accepts(true, 0, 0, 10000));
    assert(!altitude_filter_accepts(true, 0, 5000, 10000));
    assert(altitude_filter_accepts(false, 40000, 0, 0));
    puts("PASS snapshot/altitude boundary tests");
}
