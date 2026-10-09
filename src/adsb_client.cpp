// Fetch nearby aircraft from airplanes.live (fallback adsb.lol) and parse the
// readsb JSON into a vector<Aircraft>.
//
// Memory safety (important on the ESP32): we parse straight from the HTTP stream
// (no full-body String), use an ArduinoJson field filter so only the ~12 fields we
// need are kept, and hard-cap the number of aircraft (ADSB_MAX_AIRCRAFT). The radar
// then keeps only the nearest ~20 for display.
#include "adsb_client.h"
#include "config.h"
#include "geo.h"           // haversineKm — keep the nearest N aircraft
#include "altitude_filter.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>   // v7
#include <esp_heap_caps.h>

// Parse the JSON in PSRAM, not internal RAM. Otherwise the per-poll JSON alloc/free
// churn fragments the internal heap and, after a while, mbedTLS can't find a large
// enough contiguous block for the TLS handshake (-32512), freezing the feed.
struct PsramJsonAllocator : ArduinoJson::Allocator {
    void* allocate(size_t n) override { return heap_caps_malloc(n, MALLOC_CAP_SPIRAM); }
    void  deallocate(void* p) override { heap_caps_free(p); }
    void* reallocate(void* p, size_t n) override { return heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM); }
};
static PsramJsonAllocator s_jsonPsram;

void AdsbClient::begin(double homeLat, double homeLon, float rangeKm) {
    _lat = homeLat; _lon = homeLon; _rangeKm = rangeKm;
}

struct AdsbProvider {const char* host; const char* path;};
static const AdsbProvider providers[ADSB_PROVIDER_COUNT]={
 {ADSB_PRIMARY_HOST,"/v2/point/%.4f/%.4f/%.0f"},
 {ADSB_OPENDATA_HOST,"/api/v3/lat/%.4f/lon/%.4f/dist/%.0f"},
 {ADSB_FALLBACK_HOST,"/v2/point/%.4f/%.4f/%.0f"}
};

bool AdsbClient::poll(std::vector<Aircraft>& out) {
    if (WiFi.status()!=WL_CONNECTED) return false;
    bool attempted=false;
    for (int i=0;i<ADSB_PROVIDER_COUNT;++i) {
        if (_pacer.cooling(i,millis())) continue;
        attempted=true;
        if(fetchFrom(i,out)){_lastPollSkipped=false;return true;}
    }
    _lastPollSkipped=!attempted;
    return false;
}

// Stream::readBytes retries transient empty TLS reads until timeout.
class JsonNetworkStream : public Stream {
public:
 explicit JsonNetworkStream(Stream& s):src(s){}
 int available() override{return src.available();}
 int read() override{return src.read();}
 int peek() override{return src.peek();}
 void flush() override{src.flush();}
 size_t write(uint8_t) override{return 0;}
private: Stream& src;
};

bool AdsbClient::fetchFrom(int slot, std::vector<Aircraft>& out) {
    const char* host=providers[slot].host;
    const double nm=_rangeKm*0.539957;
    char path[100],url[180];
    snprintf(path,sizeof(path),providers[slot].path,_lat,_lon,nm);
    snprintf(url,sizeof(url),"https://%s%s",host,path);
    WiFiClientSecure client;
#if ADSB_HTTPS_INSECURE
    client.setInsecure();
#endif
    client.setHandshakeTimeout(TLS_HANDSHAKE_S);
    HTTPClient http;
    http.setReuse(false);
    // Chunked JSON is not a plain socket JSON stream. HTTP/1.0 avoids chunk encoding.
    http.useHTTP10(true);
    http.setConnectTimeout(6000);
    http.setTimeout(8000);
    _pacer.onAttempt(slot,millis());
    if(!http.begin(client,url)) {Serial.printf("[adsb] begin failed: %s\n",host);return false;}
    http.setUserAgent(ADSB_USER_AGENT);
    http.addHeader("Accept","application/json");
    const char* names[]={"Retry-After"};
    http.collectHeaders(names,1);
    int code=http.GET();
    if(code>0) _lastResponseMs=millis();
    if(code!=200){
        Serial.printf("[adsb] HTTP %d (%s)\n",code,host);
        if(code==403)_pacer.onRefused(slot,millis());
        else if(code==429)_pacer.onLimited(slot,millis(),http.header("Retry-After").toInt());
        else if(code>0)_pacer.onUnusable(slot);
        http.end();return false;
    }
    // Only keep the fields we use -> much smaller parsed document.
    JsonDocument filter(&s_jsonPsram);
    const char* keys[] = { "ac", "aircraft" };
    const char* flds[] = { "hex", "flight", "t", "lat", "lon", "alt_baro",
                           "track", "true_heading", "gs", "baro_rate",
                           "squawk", "seen_pos", "dbFlags" };
    for (const char* k : keys)
        for (const char* f : flds)
            filter[k][0][f] = true;

    JsonDocument doc(&s_jsonPsram);
    JsonNetworkStream response(http.getStream());
    response.setTimeout(8000);
    DeserializationError err = deserializeJson(doc, response,
                                               DeserializationOption::Filter(filter));
    http.end();
    if (err) { Serial.printf("[adsb] JSON error %s (%s)\n",err.c_str(),host);_pacer.onUnusable(slot);return false; }

    JsonArrayConst arr = doc["ac"].as<JsonArrayConst>();
    if (arr.isNull()) arr = doc["aircraft"].as<JsonArrayConst>();
    if (arr.isNull()) {Serial.printf("[adsb] no aircraft array (%s)\n",host);_pacer.onUnusable(slot);return false;}
    _lastHost=host;
    _pacer.onOk(slot);

    // Keep the ADSB_MAX_AIRCRAFT *nearest* aircraft (not just the first ones the feed happens to
    // list), so busy areas still show the traffic closest to you. We gate by distance BEFORE
    // parsing the strings, so the hundreds of far-away aircraft never allocate anything.
    std::vector<Aircraft> tmp;
    std::vector<float>     dist;             // parallel array: km from home for each kept aircraft
    tmp.reserve(ADSB_MAX_AIRCRAFT);
    dist.reserve(ADSB_MAX_AIRCRAFT);
    const uint32_t now = millis();
    for (JsonObjectConst a : arr) {
        if (a["lat"].isNull() || a["lon"].isNull()) continue;   // need a position
        const double lat = a["lat"].as<double>();
        const double lon = a["lon"].as<double>();

        // alt_baro is the string "ground" for aircraft on the ground; skip them if hide-ground is on.
        const bool  onGround = a["alt_baro"].is<const char*>();
        const float altFt    = onGround ? 0.0f : (a["alt_baro"] | 0.0f);
        if (_hideGround && onGround) continue;
        // optional filters (applied before the cap, so slots only go to matching aircraft)
        if (!altitude_filter_accepts(onGround, altFt, _minAltFt, _maxAltFt)) continue;
        if (_milOnly && (((a["dbFlags"] | 0u) & 0x1) == 0)) continue;

        const float d = (float)geo::haversineKm(_lat, _lon, lat, lon);

        // nearest-N gate: if the buffer is full and this one isn't closer than the farthest kept,
        // drop it now — before any string allocation.
        int farIdx = -1;
        if ((int)tmp.size() >= ADSB_MAX_AIRCRAFT) {
            farIdx = 0;
            for (int i = 1; i < (int)dist.size(); ++i) if (dist[i] > dist[farIdx]) farIdx = i;
            if (d >= dist[farIdx]) continue;
        }

        Aircraft ac;
        ac.hex = (const char*)(a["hex"] | "");
        if (ac.hex.length() == 0) continue;
        ac.flight = String((const char*)(a["flight"] | "")); ac.flight.trim();
        ac.type   = (const char*)(a["t"] | "");
        ac.lat = lat; ac.lon = lon;
        ac.onGround = onGround;
        ac.altBaro  = altFt;
        ac.track    = a["track"].is<float>() ? a["track"].as<float>() : (a["true_heading"] | NAN);
        ac.gs       = a["gs"] | NAN;
        ac.baroRate = a["baro_rate"] | NAN;
        ac.squawk   = a["squawk"].is<const char*>() ? atoi(a["squawk"]) : (a["squawk"] | -1);
        ac.seenPos  = a["seen_pos"] | 0;
        ac.military = ((a["dbFlags"] | 0u) & 0x1) != 0;
        ac.lastUpdateMs = now;

        if (farIdx >= 0) { tmp[farIdx] = std::move(ac); dist[farIdx] = d; }   // replace the farthest kept
        else             { tmp.push_back(std::move(ac)); dist.push_back(d); }
    }

    out.swap(tmp);
    _lastOkMs = now;
    return true;
}
