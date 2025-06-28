#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "GeoLocation.h"
#include "DateTime.h"
#include "settings.h"

#define FETCH_PERIOD (30 * 60 * 1000) // 30 minutes
#define RETRY_PERIOD (3 * 1000)       // 3 seconds

namespace service
{
    void GeoLocation::begin()
    {
        settings::data().init(db::geo_method, int(Method::IPAddress));
        settings::data().init(db::geo_latitude,  50.4500f);
        settings::data().init(db::geo_longitude, 30.5233f);
        settings::data().init(db::geo_timezone,  200);
        startRequest();
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        b.beginGuest();
        sets::Group g(b, "📍 Geolocation");

        auto options = "Manual;IP Address (ipapi.co);WiFi Stations (Google)";
        if (b.Select(db::geo_method, "Method", options))
        {
            startRequest();
            b.reload();
            return;
        }

        if (getMethod() == Method::Manual)
        {
            b.Number(db::geo_latitude, "Latitude", nullptr, -90.f, +90.f);
            b.Number(db::geo_longitude, "Longitude", nullptr, -180.f, +180.f);

            int tzh, tzm;
            decodeTimeZone(tzh, tzm);
            int num = (tzm == 0 ? tzh : tzh * 60 + tzm);
            if (b.Number("Time zone", &num)) setTimeZone(num);
        }
        else
        {
            b.LabelFloat("latitude"_h, "Latitude", getLatitude(), 4);
            b.LabelFloat("longitude"_h, "Longitude", getLongitude(), 4);
            b.LabelNum("timezone"_h, "Time zone", getTimeZone());
        }
        b.endGuest();
    }

    void GeoLocation::settingsUpdate(sets::Updater &u)
    {
        if (getMethod() != Method::Manual)
        {
            u.update("latitude"_h, getLatitude(), 4);
            u.update("longitude"_h, getLongitude(), 4);
            u.update("timezone"_h, getTimeZone());
        }
    }

    GeoLocation::Method GeoLocation::getMethod() const
    {
        return Method(int(settings::data()[db::geo_method]));
    }

    float GeoLocation::getLatitude() const
    {
        return settings::data()[db::geo_latitude];
    }

    float GeoLocation::getLongitude() const
    {
        return settings::data()[db::geo_longitude];
    }

    int GeoLocation::getTZOffset() const
    {
        int tzh, tzm;
        decodeTimeZone(tzh, tzm);
        return (tzh * 60 + tzm) * 60;
    }

    ////////////////////////////////////////////////////////////////////////////

    void GeoLocation::startRequest()
    {
        Task::stop();
        Task::start("geo_location");
    }

    void GeoLocation::task()
    {
        while (!requestGeoLocation()) sleep(RETRY_PERIOD);
    }

    String GeoLocation::getTimeZone() const
    {
        int tzh, tzm;
        decodeTimeZone(tzh, tzm);
        char buffer[6];
        if (tzm == 0) sprintf(buffer, "%+d", tzh);
        else sprintf(buffer, "%+d:%02d", tzh, tzm);
        return String(buffer);
    }

    void GeoLocation::setTimeZone(int num)
    {
        int tzh, tzm;
        if (num >= -14 && num <= +12)
            tzh = num, tzm = 0;
        else
            tzh = num / 60, tzm = abs(num) % 60;
        encodeTimeZone(tzh, tzm);
    }

    void GeoLocation::encodeTimeZone(int &tzh, int &tzm) const
    {
        int off = (tzh * 100 + tzm);
        settings::data()[db::geo_timezone ] = off;
    }

    void GeoLocation::decodeTimeZone(int &tzh, int &tzm) const
    {
        int off = settings::data()[db::geo_timezone];
        tzh = off / 100, tzm = abs(off) % 100;
    }

    bool GeoLocation::requestGeoLocation()
    {
        float lat, lon; // location
        int   tzh, tzm; // timezone

        bool ok = false;
        switch (getMethod())
        {
            case Method::Manual:
                lat = getLatitude();
                lon = getLongitude();
                decodeTimeZone(tzh, tzm);
                ok = true;
                break;

            case Method::IPAddress: 
                ok = requestUsingIPAddress(lat, lon, tzh, tzm);
                break;

            case Method::WiFiStations:
                ok = requestUsingWiFiStations(lat, lon, tzh, tzm);
                break;
        }
        if (ok)
        {
            settings::data()[db::geo_latitude ] = lat;
            settings::data()[db::geo_longitude] = lon;
            encodeTimeZone(tzh, tzm);
            settings::data().update();
            log_i("request geolocation SUCCESS!");
        }
        else
        {
            log_i("request geolocation FAILED!");
        }
        return ok;
    }

    bool GeoLocation::requestUsingIPAddress(float& lat, float& lon, int& tzh, int& tzm) 
    {
        HTTPClient http;
        if (http.begin("https://ipapi.co/json/")) 
        {
            if (http.GET() == HTTP_CODE_OK) 
            {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, http.getString());
                if (!error)
                {
                    lat = doc["latitude"].as<float>();
                    lon = doc["longitude"].as<float>();
                    int off = doc["utc_offset"].as<int>();
                    tzh = off / 100, tzm = abs(off) % 100;
                    http.end();
                    return true;
                }                
            }
            http.end();
        }
        return false;
    }

    bool GeoLocation::requestUsingWiFiStations(float &lat, float &lon, int &tzh, int &tzm)
    {
        bool result = false;
        WiFi.scanNetworks();
        if (requestGoogleGeolocationApi(lat, lon))
        {
            long timestamp = service::dateTime.getNow();
            result = requestGoogleTimeZoneApi(lat, lon, timestamp, tzh, tzm);
        }
        WiFi.scanDelete();
        return result;
    }

    bool GeoLocation::requestGoogleGeolocationApi(float &lat, float &lon)
    {
        auto found = min(int(WiFi.scanComplete()), 5);
        if (found <= 0) return false;

        JsonDocument doc;
        JsonArray aps = doc["wifiAccessPoints"].to<JsonArray>();
        for (int i = 0; i < found; ++i)
        {
            JsonObject ap = aps.add<JsonObject>();
            ap["macAddress"] = WiFi.BSSIDstr(i);
            ap["signalStrength"] = WiFi.RSSI(i);
        }

        char payload[512];
        auto payloadLen = serializeJson(doc, payload, sizeof(payload));
        auto key = settings::apikey(db::apikey_google);
        auto url = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + key;

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        if (http.POST((uint8_t*)payload, payloadLen) == HTTP_CODE_OK) 
        {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, http.getString());
            if (!error)
            {
                lat = doc["location"]["lat"].as<float>();
                lon = doc["location"]["lng"].as<float>();
                http.end();
                return true;
            }
        }
        http.end();
        return false;
    }

    bool GeoLocation::requestGoogleTimeZoneApi(float lat, float lon, long timestamp, int &tzh, int &tzm)
    {
        auto key = settings::apikey(db::apikey_google);
        auto url = String("https://maps.googleapis.com/maps/api/timezone/json?location=")
            + String(lat, 6) + "," + String(lon, 6)
            + "&timestamp=" + String(timestamp)
            + "&key=" + key;

        HTTPClient http;
        http.begin(url);
        if (http.GET() == HTTP_CODE_OK) 
        {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, http.getString());
            if (!error)
            {
                auto raw = doc["rawOffset"].as<long>();
                auto dst = doc["dstOffset"].as<long>();
                auto off  = raw + dst;
                tzh = off / 3600;
                tzm = (abs(off) % 3600) / 60;
                http.end();
                return true;
            }
        }
        http.end();
        return false;
    }

    GeoLocation geoLocation;
}
