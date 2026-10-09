#include "weather.h"
#include <mutex>
#include <string.h>
#include <stdio.h>
#include <time.h>
static std::mutex m;
static WeatherSnapshot data;
void weather_store(const WeatherSnapshot &w) { std::lock_guard<std::mutex> lock(m); data=w; }
bool weather_get(WeatherSnapshot &w) { std::lock_guard<std::mutex> lock(m); w=data; return w.valid; }
const char *weather_condition(int c) {
    if(c==0) return "Clear";
    if(c<=3 && c>=1) return "Cloudy";
    if(c==45||c==48) return "Fog";
    if(c>=51&&c<=57) return "Drizzle";
    if(c>=61&&c<=67) return "Rain";
    if(c>=71&&c<=77) return "Snow";
    if(c>=80&&c<=82) return "Showers";
    if(c==85||c==86) return "Snow showers";
    if(c>=95&&c<=99) return "Thunderstorm";
    return "Weather";
}
const char *weather_day_name(const char *iso) {
    static const char *names[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    if(!iso || strlen(iso)<10) return "---";
    struct tm d={}; int y=0,mo=0,da=0;
    if(sscanf(iso,"%d-%d-%d", &y,&mo,&da)!=3) return "---";
    d.tm_year=y-1900; d.tm_mon=mo-1; d.tm_mday=da; d.tm_hour=12;
    // mktime uses configured TZ but weekday is correct for dates at local noon.
    if(mktime(&d)==(time_t)-1) return "---";
    return names[d.tm_wday];
}
