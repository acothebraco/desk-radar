#include "weather_client.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>

bool weather_fetch(double lat, double lon, WeatherSnapshot &out) {
    if (WiFi.status()!=WL_CONNECTED) return false;
    char url[550];
    snprintf(url,sizeof(url),
        "https://api.open-meteo.com/v1/forecast?latitude=%.5f&longitude=%.5f"
        "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m"
        "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max"
        "&forecast_days=4&timezone=auto",lat,lon);
    WiFiClientSecure client; client.setInsecure(); client.setHandshakeTimeout(TLS_HANDSHAKE_S);
    HTTPClient http; http.setReuse(false); http.setConnectTimeout(3500); http.setTimeout(7500);
    if(!http.begin(client,url)) return false;
    http.setUserAgent(ADSB_USER_AGENT);
    const int code=http.GET();
    if(code!=HTTP_CODE_OK) { Serial.printf("[weather] HTTP %d\n",code); http.end(); return false; }
    // getString() strips chunked transfer framing unlike parsing getStream() directly.
    String body=http.getString(); http.end();
    if(body.length()==0 || body.length()>24576) return false;
    JsonDocument doc;
    if(deserializeJson(doc,body)) return false;
    JsonObjectConst current=doc["current"].as<JsonObjectConst>();
    JsonObjectConst daily=doc["daily"].as<JsonObjectConst>();
    if(current.isNull()||daily.isNull()) return false;
    WeatherSnapshot w={};
    const char *ts=current["time"]|"";
    const char *clock=strchr(ts,'T');
    snprintf(w.updated,sizeof(w.updated),"%.5s",clock ? clock+1 : "--:--");
    w.code=current["weather_code"] | -1;
    w.tempC=current["temperature_2m"]|0.0f;
    w.feelsC=current["apparent_temperature"]|w.tempC;
    w.humidity=current["relative_humidity_2m"]|0;
    w.windKmh=current["wind_speed_10m"]|0.0f;
    w.windDeg=current["wind_direction_10m"]|0;
    JsonArrayConst dates=daily["time"].as<JsonArrayConst>();
    JsonArrayConst codes=daily["weather_code"].as<JsonArrayConst>();
    JsonArrayConst high=daily["temperature_2m_max"].as<JsonArrayConst>();
    JsonArrayConst low=daily["temperature_2m_min"].as<JsonArrayConst>();
    JsonArrayConst rain=daily["precipitation_probability_max"].as<JsonArrayConst>();
    w.dayCount=dates.size()<4?(int)dates.size():4;
    for(int i=0;i<w.dayCount;i++) {
        snprintf(w.days[i].date,sizeof(w.days[i].date),"%s",dates[i]|"");
        w.days[i].code=codes[i]|-1;
        w.days[i].highC=high[i]|0.0f;
        w.days[i].lowC=low[i]|0.0f;
        w.days[i].rainChance=rain[i]|0;
    }
    w.valid=true; out=w; return true;
}
