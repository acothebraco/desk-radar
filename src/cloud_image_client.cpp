#include "cloud_image_client.h"
#include "weather_image.h"
#include "net_fetch.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <TJpg_Decoder.h>
#include <esp_heap_caps.h>
#include <time.h>
static uint16_t *dst=nullptr;
static bool jpg_block(int16_t x,int16_t y,uint16_t w,uint16_t h,uint16_t *bmp) {
    if(!dst)return false;
    const int c=WX_IMAGE_SIZE/2;
    for(int j=0;j<(int)h;j++) {
        int yy=y+j;if(yy<0||yy>=WX_IMAGE_SIZE)continue;
        for(int i=0;i<(int)w;i++) {
            int xx=x+i;if(xx<0||xx>=WX_IMAGE_SIZE)continue;
            const int dx=xx-c,dy=yy-c;
            dst[yy*WX_IMAGE_SIZE+xx]=(dx*dx+dy*dy <= (c-2)*(c-2))?bmp[j*w+i]:0;
        }
    }
    return true;
}
bool cloud_image_fetch(double lat,double lon) {
    if(WiFi.status()!=WL_CONNECTED || !(dst=weather_image_back(WX_CLOUDS)))return false;
    // EPSG:4326 WMS 1.1.1 uses longitude,latitude ordering.
    char url[760];
    snprintf(url,sizeof(url),
      "https://view.eumetsat.int/geoserver/wms?service=WMS&version=1.1.1&request=GetMap"
      "&layers=mtg_fd%%3Argb_cloudtype%%2Cbackgrounds%%3Ane_10m_coastline%%2Cbackgrounds%%3Ane_boundary_lines_land"
      "&srs=EPSG%%3A4326&bbox=%.5f%%2C%.5f%%2C%.5f%%2C%.5f&width=360&height=360"
      "&styles=&format=image%%2Fjpeg&bgcolor=0x000000",
      lon-3.0,lat-2.0,lon+3.0,lat+2.0);
    uint8_t *src=nullptr;size_t len=0;
    if(!net_fetch_psram(url,ADSB_USER_AGENT,&src,&len,180000,4500,12000))return false;
    uint16_t jw=0,jh=0;
    if(TJpgDec.getJpgSize(&jw,&jh,src,(uint32_t)len)!=JDR_OK || jw!=WX_IMAGE_SIZE || jh!=WX_IMAGE_SIZE) {
        heap_caps_free(src); return false;
    }
    TJpgDec.setJpgScale(1); TJpgDec.setSwapBytes(false);
    TJpgDec.setCallback(jpg_block);
    const JRESULT status=TJpgDec.drawJpg(0,0,src,(uint32_t)len);
    heap_caps_free(src);
    dst=nullptr;
    if(status!=JDR_OK)return false;
    weather_image_commit(WX_CLOUDS,(uint32_t)time(nullptr),lat,lon);
    Serial.println("[clouds] EUMETSAT image ready");
    return true;
}
