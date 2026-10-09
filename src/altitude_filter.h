#pragma once
// Evaluate altitude limits before spending memory on an aircraft object.
// An upper limit does not exclude ground vehicles; a lower limit excludes ground.
inline bool altitude_filter_accepts(bool onGround, float altFt,
                                    float minAltFt, float maxAltFt) {
    if (minAltFt > 0.0f && (onGround || altFt < minAltFt)) return false;
    if (maxAltFt > 0.0f && !onGround && altFt > maxAltFt) return false;
    return true;
}
