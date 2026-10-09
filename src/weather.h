#pragma once
struct WeatherDay {
    char date[11]; int code; float lowC, highC; int rainChance;
};
struct WeatherSnapshot {
    bool valid = false;
    char updated[6] = {};
    int code = -1; float tempC = 0, feelsC = 0, windKmh = 0;
    int humidity = 0, windDeg = 0;
    WeatherDay days[4] = {};
    int dayCount = 0;
};
void weather_store(const WeatherSnapshot &w);
bool weather_get(WeatherSnapshot &w);
const char *weather_condition(int code);
const char *weather_day_name(const char *iso);
