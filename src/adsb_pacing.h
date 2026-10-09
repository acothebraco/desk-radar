#pragma once
#include <stdint.h>
#include "config.h"
// Independent, rollover-safe provider pacing. Never retry policy refusals in a tight loop.
class AdsbPacer {
public:
 bool cooling(int i, uint32_t now) const {
   if ((int32_t)(until[i]-now)>0) return true;
   return spacing[i] && (int32_t)(now-attempt[i]) < (int32_t)spacing[i];
 }
 void onAttempt(int i,uint32_t now){attempt[i]=now;}
 void onRefused(int i,uint32_t now){
   uint32_t park=park403[i]?park403[i]:ADSB_COOLDOWN_403_MS;
   until[i]=now+park;
   park403[i]=park>=ADSB_COOLDOWN_403_MAX_MS/2?ADSB_COOLDOWN_403_MAX_MS:park*2;
 }
 void onLimited(int i,uint32_t now,long seconds){
   if(seconds>0) until[i]=now+(uint32_t)seconds*1000UL;
   onUnusable(i);
 }
 void onUnusable(int i){
   okStreak[i]=0;
   uint32_t next=spacing[i]?spacing[i]*2:ADSB_SPACING_STEP_MS;
   spacing[i]=next>ADSB_SPACING_MAX_MS?ADSB_SPACING_MAX_MS:next;
 }
 void onOk(int i){
   park403[i]=0;
   if(spacing[i] && ++okStreak[i]>=ADSB_SPACING_EASE_OKS){
     okStreak[i]=0;
     spacing[i]=spacing[i]>ADSB_SPACING_STEP_MS?spacing[i]-ADSB_SPACING_STEP_MS:0;
   }
 }
private:
 uint32_t until[ADSB_PROVIDER_COUNT]={0};
 uint32_t park403[ADSB_PROVIDER_COUNT]={0};
 uint32_t spacing[ADSB_PROVIDER_COUNT]={0};
 uint32_t attempt[ADSB_PROVIDER_COUNT]={0};
 uint16_t okStreak[ADSB_PROVIDER_COUNT]={0};
};
