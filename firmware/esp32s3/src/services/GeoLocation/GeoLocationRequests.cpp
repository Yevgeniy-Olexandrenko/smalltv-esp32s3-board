#include "GeoLocationRequests.h"
#include "services/DateTime.h"
#include "services/WiFiConnection.h"
#include "settings/Settings.h"

namespace service::details
{
    bool GeoLocationRequests::requestUsingIPAddress(float& lat, float& lon, int& tzh, int& tzm) 
    {
        const char URL[] = "https://ipapi.co/json/";
        if (m_http.begin(URL)) 
        {
            m_http.addHeader("User-Agent", service::wifiConnection.getUserAgent());
            if (m_http.GET() == HTTP_CODE_OK) 
            {
                m_json.clear();
                DeserializationError error = deserializeJson(m_json, m_http.getString());
                if (!error)
                {
                    lat = m_json["latitude"].as<float>();
                    lon = m_json["longitude"].as<float>();
                    int off = m_json["utc_offset"].as<int>();
                    tzh = off / 100, tzm = abs(off) % 100;
                    m_http.end();
                    return true;
                }                
            }
        }
        m_http.end();
        return false;
    }

    bool GeoLocationRequests::requestUsingWiFiStations(float& lat, float& lon, int& tzh, int& tzm)
    {
        if (Settings::apikey(apikey::google).length() >= 39 && service::dateTime.isSynced())
        {
            if (WiFi.scanComplete() < 1) 
            {
                WiFi.scanNetworks(false, true);
            }
            if (requestGoogleGeolocationApi(lat, lon))
            {
                long timestamp = service::dateTime.getNow();
                if (requestGoogleTimeZoneApi(lat, lon, timestamp, tzh, tzm))
                {
                    WiFi.scanDelete();
                    return true;
                }
            }
        }
        return false;
    }

    bool GeoLocationRequests::requestReverseGeocoding(float lat, float lon, String& locality, String& countryCode)
    {
        const char URL[] =
            "https://nominatim.openstreetmap.org/reverse"
            "?lat=%.6f&lon=%.6f&format=json&addressdetails=1&zoom=13";

        locality.clear();
        countryCode.clear();

        char url[128];
        snprintf(url, sizeof(url), URL, lat, lon);

        if (m_http.begin(url)) 
        {
            m_http.addHeader("User-Agent", service::wifiConnection.getUserAgent());
            m_http.addHeader("Accept-Language", "en");
            if (m_http.GET() == HTTP_CODE_OK) 
            {
                m_json.clear();
                DeserializationError error = deserializeJson(m_json, m_http.getString());
                if (!error)
                {
                    JsonObject address = m_json["address"];
                    if (address)
                    {
                        for (const char* key : { "city", "town", "village", "hamlet", "municipality" })
                        {
                            if (address[key])
                            { 
                                locality = address[key].as<String>(); 
                                break; 
                            }
                        }

                        auto isASCII = [](const String& str)
                        {
                            for (auto s = str.c_str(); *s; ++s)
                                if ((unsigned char)(*s) & 0x80) return false;
                            return true;
                        };

                        if (locality.isEmpty() || !isASCII(locality))
                        {
                            locality = address["country"].as<String>();
                        }
                        countryCode = address["country_code"].as<String>();
                        countryCode.toUpperCase();
                    }
                }                
            }
        }

        log_i("country:  %s", countryCode.c_str());
        log_i("locality: %s", locality.c_str());

        m_http.end();
        return (!locality.isEmpty() && countryCode.length() == 2);
    }

    bool GeoLocationRequests::requestGoogleGeolocationApi(float& lat, float& lon)
    {
        const char URL[] = 
            "https://www.googleapis.com/geolocation/v1/geolocate"
            "?key=%s";

        auto found = min(int(WiFi.scanComplete()), 8);
        if (found <= 0) return false;

        m_json.clear();
        JsonArray aps = m_json["wifiAccessPoints"].to<JsonArray>();
        for (int i = 0; i < found; ++i)
        {
            JsonObject ap = aps.add<JsonObject>();
            ap["macAddress"] = WiFi.BSSIDstr(i);
            ap["signalStrength"] = WiFi.RSSI(i);
        }

        char payload[512];
        auto payloadLen = serializeJson(m_json, payload, sizeof(payload));

        log_i("wifi_found:   %d", found);
        log_i("payload_size: %d", payloadLen);
        log_i("payload_text: %s", payload);

        char url[128];
        snprintf(url, sizeof(url), URL, Settings::apikey(apikey::google).c_str());

        if (m_http.begin(url))
        {
            m_http.addHeader("User-Agent", service::wifiConnection.getUserAgent());
            m_http.addHeader("Content-Type", "application/json");

            if (m_http.POST((uint8_t*)payload, payloadLen) == HTTP_CODE_OK) 
            {
                m_json.clear();
                DeserializationError error = deserializeJson(m_json, m_http.getString());
                if (!error)
                {
                    lat = m_json["location"]["lat"].as<float>();
                    lon = m_json["location"]["lng"].as<float>();
                    m_http.end();
                    return true;
                }
            }
        }
        m_http.end();
        return false;
    }

    bool GeoLocationRequests::requestGoogleTimeZoneApi(float lat, float lon, long timestamp, int& tzh, int& tzm)
    {
        const char URL[] = 
            "https://maps.googleapis.com/maps/api/timezone/json"
            "?location=%.6f,%.6f&timestamp=%ld&key=%s";

        char url[192];
        snprintf(url, sizeof(url), URL, lat, lon, timestamp, Settings::apikey(apikey::google).c_str());

        if (m_http.begin(url))
        {
            m_http.addHeader("User-Agent", service::wifiConnection.getUserAgent());
            if (m_http.GET() == HTTP_CODE_OK) 
            {
                m_json.clear();
                DeserializationError error = deserializeJson(m_json, m_http.getString());
                if (!error)
                {
                    auto raw = m_json["rawOffset"].as<long>();
                    auto dst = m_json["dstOffset"].as<long>();
                    auto off = raw + dst;
                    tzh = off / 3600;
                    tzm = (abs(off) % 3600) / 60;
                    m_http.end();
                    return true;
                }
            }
        }
        m_http.end();
        return false;
    }
}
