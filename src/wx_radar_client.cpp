#include "wx_radar_client.h"
#include "weather_image.h"
#include "net_fetch.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PNGdec.h>
#include <esp_heap_caps.h>
#include <new>
#include <string.h>
static PNG *decoder=nullptr;
static uint16_t *pixels=nullptr;
static bool ensure_png() {
    if(decoder) return true;
    void *m=heap_caps_malloc(sizeof(PNG),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if(!m)return false;
    decoder=new(m)PNG(); return true;
}
static int png_line(PNGDRAW *draw) {
    if(!pixels || draw->iWidth!=WX_TILE_SIZE)return 1;
    const int cut=(WX_TILE_SIZE-WX_IMAGE_SIZE)/2;
    if(draw->y<cut || draw->y>=cut+WX_IMAGE_SIZE)return 1;
    uint16_t line[WX_TILE_SIZE];
    if(draw->iPixelType==PNG_PIXEL_TRUECOLOR_ALPHA && draw->iBpp==8) {
        const uint8_t *q=draw->pPixels;
        for(int x=0;x<WX_TILE_SIZE;x++,q+=4)
            line[x]=(uint16_t)(((q[0]>>3)<<11)|((q[1]>>2)<<5)|(q[2]>>3));
    } else decoder->getLineAsRGB565(draw,line,PNG_RGB565_LITTLE_ENDIAN,0);
    const int outY=draw->y-cut,cy=WX_IMAGE_SIZE/2;
    for(int x=0;x<WX_IMAGE_SIZE;x++) {
        const int dx=x-cy,dy=outY-cy;
        pixels[outY*WX_IMAGE_SIZE+x]=(dx*dx+dy*dy <= (cy-2)*(cy-2))?line[x+cut]:0;
    }
    return 1;
}
bool wx_radar_fetch(double lat,double lon) {
    if(WiFi.status()!=WL_CONNECTED || !(pixels=weather_image_back(WX_PRECIP)) || !ensure_png())return false;
    WiFiClientSecure cli;cli.setInsecure();cli.setHandshakeTimeout(TLS_HANDSHAKE_S);
    HTTPClient http;http.setReuse(false);http.setConnectTimeout(3500);http.setTimeout(6500);
    if(!http.begin(cli,"https://api.rainviewer.com/public/weather-maps.json"))return false;
    http.setUserAgent(ADSB_USER_AGENT);
    if(http.GET()!=HTTP_CODE_OK){http.end();return false;}
    String meta=http.getString();http.end();
    JsonDocument doc;
    if(deserializeJson(doc,meta))return false;
    const char *host=doc["host"]|"";
    JsonArrayConst past=doc["radar"]["past"].as<JsonArrayConst>();
    if(!host[0]||past.isNull()||!past.size())return false;
    JsonObjectConst last=past[past.size()-1].as<JsonObjectConst>();
    const char *path=last["path"]|""; uint32_t ts=last["time"]|0;
    if(!path[0])return false;
    char url[360];
    snprintf(url,sizeof(url),"%s%s/512/7/%.5f/%.5f/2/1_1.png",host,path,lat,lon);
    uint8_t *src=nullptr;size_t len=0;
    if(!net_fetch_psram(url,ADSB_USER_AGENT,&src,&len,260000,3500,8500))return false;
    memset(pixels,0,(size_t)WX_IMAGE_SIZE*WX_IMAGE_SIZE*sizeof(uint16_t));
    const int rc=decoder->openRAM(src,(int)len,png_line);
    bool ok=false;
    if(rc==PNG_SUCCESS) {
        if(decoder->getWidth()==WX_TILE_SIZE && decoder->getHeight()==WX_TILE_SIZE)
            ok=(decoder->decode(nullptr,0)==PNG_SUCCESS);
        decoder->close();
    }
    heap_caps_free(src);
    if(ok) {weather_image_commit(WX_PRECIP,ts,lat,lon);Serial.printf("[wxradar] image ready frame=%lu\n",(unsigned long)ts);}
    else Serial.printf("[wxradar] PNG decoder error=%d\n",rc);
    return ok;
}
