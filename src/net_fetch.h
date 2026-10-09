#pragma once
#include <stddef.h>
#include <stdint.h>
// On success, *out is owned by the caller and must be heap_caps_free()'d.
bool net_fetch_psram(const char *url, const char *agent, uint8_t **out,
                     size_t *outLen, size_t maxLen, int connectMs, int totalMs);
