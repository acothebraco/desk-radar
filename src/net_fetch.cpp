#include "net_fetch.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_heap_caps.h>

bool net_fetch_psram(const char *url,const char *agent,uint8_t **out,
                     size_t *outLen,size_t maxLen,int connectMs,int totalMs) {
    *out=nullptr; *outLen=0;
    if(WiFi.status()!=WL_CONNECTED) return false;
    WiFiClientSecure cli; cli.setInsecure(); cli.setHandshakeTimeout(TLS_HANDSHAKE_S);
    HTTPClient http; http.setReuse(false); http.setConnectTimeout(connectMs); http.setTimeout(totalMs);
    if(!http.begin(cli,url)) return false;
    if(agent) http.setUserAgent(agent);
    const int code=http.GET();
    if(code!=HTTP_CODE_OK) {Serial.printf("[weather-net] HTTP %d\n",code);http.end();return false;}
    const int len=http.getSize();
    uint8_t *buf=nullptr; size_t got=0;
    if(len>0) {
        // Never pass truncated images to decoders.
        if((size_t)len>maxLen) {http.end();return false;}
        buf=(uint8_t*)heap_caps_malloc((size_t)len,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        if(!buf) {http.end();return false;}
        WiFiClient *stream=http.getStreamPtr(); uint32_t last=millis();
        while(got<(size_t)len && millis()-last<(uint32_t)totalMs) {
            size_t available=stream->available();
            if(available) {
                size_t toRead=available<(size_t)len-got?available:(size_t)len-got;
                int n=stream->readBytes(buf+got,toRead);
                if(n>0){got+=(size_t)n;last=millis();}
            } else if(!http.connected()) break;
            else delay(5);
        }
    } else {
        // HTTPClient decodes chunked replies here. Limit is enforced before copying.
        String body=http.getString();
        if(body.length()>0 && body.length()<=maxLen) {
            got=body.length(); buf=(uint8_t*)heap_caps_malloc(got,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
            if(buf) memcpy(buf,body.c_str(),got); else got=0;
        }
    }
    http.end();
    if(got==0 || (len>0 && got!=(size_t)len)) {if(buf)heap_caps_free(buf);return false;}
    *out=buf;*outLen=got;return true;
}
