#pragma once
#include <stdint.h>
#define WX_IMAGE_SIZE 360
#define WX_TILE_SIZE 512
enum WeatherImageKind { WX_PRECIP=0, WX_CLOUDS=1 };
// Core 0 writes exclusively to the back buffer and calls commit after decoding.
uint16_t *weather_image_back(WeatherImageKind kind);
void weather_image_commit(WeatherImageKind kind,uint32_t stamp,double lat,double lon);
// Core 1 copies a stable finished image into its own LVGL canvas buffer.
bool weather_image_copy(WeatherImageKind kind,uint16_t *dest,uint32_t *stamp,
                        double *lat,double *lon,uint32_t *version);
