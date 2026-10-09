#include "../src/weather_image.h"
#include "../src/weather.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main() {
    static uint16_t canvas[WX_IMAGE_SIZE*WX_IMAGE_SIZE];
    WeatherSnapshot w={};assert(!weather_get(w));
    w.valid=true;w.code=0;w.tempC=19.5f;w.dayCount=4;
    weather_store(w); WeatherSnapshot read={};
    assert(weather_get(read)); assert(read.dayCount==4 && read.tempC==19.5f);
    assert(weather_condition(0) && weather_day_name("2026-10-09"));
    uint32_t time=0,version=0;double lat=0,lon=0;
    assert(!weather_image_copy(WX_PRECIP,canvas,&time,&lat,&lon,&version));
    uint16_t *back=weather_image_back(WX_PRECIP);assert(back);
    back[0]=0x1234;back[42]=0x5678;
    weather_image_commit(WX_PRECIP,1234,52.5,13.4);
    assert(weather_image_copy(WX_PRECIP,canvas,&time,&lat,&lon,&version));
    assert(canvas[0]==0x1234 && canvas[42]==0x5678 && time==1234 && version==1);
    back=weather_image_back(WX_PRECIP);assert(back);back[0]=0x9abc;
    // No commit: previously displayed front buffer must remain unchanged.
    assert(weather_image_copy(WX_PRECIP,canvas,&time,&lat,&lon,&version));
    assert(canvas[0]==0x1234 && version==1);
    weather_image_commit(WX_PRECIP,5678,54.3,9.2);
    assert(weather_image_copy(WX_PRECIP,canvas,&time,&lat,&lon,&version));
    assert(canvas[0]==0x9abc && version==2 && time==5678);
    assert(!weather_image_copy(WX_CLOUDS,canvas,&time,&lat,&lon,&version));
    puts("Weather snapshot / PSRAM image-buffer handover PASS");
}
