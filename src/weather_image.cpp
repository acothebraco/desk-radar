#include "weather_image.h"
#include <mutex>
#include <stdlib.h>
#include <string.h>
#if defined(ESP_PLATFORM)
#include <esp_heap_caps.h>
#endif
struct ImageStore {
    uint16_t *buf[2]={nullptr,nullptr};
    int active=0; bool valid=false;
    uint32_t time=0,version=0; double lat=0,lon=0;
};
static ImageStore images[2];
static std::mutex guards[2];
static uint16_t *allocate(void) {
    const size_t n=(size_t)WX_IMAGE_SIZE*WX_IMAGE_SIZE*sizeof(uint16_t);
#if defined(ESP_PLATFORM)
    return (uint16_t*)heap_caps_malloc(n,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
#else
    return (uint16_t*)malloc(n);
#endif
}
uint16_t *weather_image_back(WeatherImageKind kind) {
    ImageStore &s=images[kind];
    // Called solely by the network task. Never swap or write the front buffer here.
    if(!s.buf[0]) s.buf[0]=allocate();
    if(!s.buf[1]) s.buf[1]=allocate();
    return s.buf[0] && s.buf[1] ? s.buf[1-s.active] : nullptr;
}
void weather_image_commit(WeatherImageKind kind,uint32_t stamp,double lat,double lon) {
    std::lock_guard<std::mutex> lock(guards[kind]);
    ImageStore &s=images[kind]; s.active=1-s.active;
    s.valid=true; s.time=stamp; s.lat=lat; s.lon=lon; s.version++;
}
bool weather_image_copy(WeatherImageKind kind,uint16_t *dest,uint32_t *stamp,
                        double *lat,double *lon,uint32_t *version) {
    std::lock_guard<std::mutex> lock(guards[kind]);
    const ImageStore &s=images[kind];
    if(!s.valid || !s.buf[s.active] || !dest) return false;
    if(*version!=s.version) {
        memcpy(dest,s.buf[s.active],(size_t)WX_IMAGE_SIZE*WX_IMAGE_SIZE*2);
        *version=s.version;
    }
    if(stamp)*stamp=s.time;
    if(lat)*lat=s.lat;
    if(lon)*lon=s.lon;
    return true;
}
