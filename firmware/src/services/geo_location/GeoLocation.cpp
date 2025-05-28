#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "GeoLocation.h"
#include "services/date_time/DateTime.h"
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
        requestNewData();
    }

    void GeoLocation::update()
    {
        if (millis() - m_fetchTS >= FETCH_PERIOD)
        {
            if (service::dateTime.isSynced() && fetchNewData())
                confirmDataReceived();
            else
                requestNewData();
        }
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        b.beginGuest();
        sets::Group g(b, "📍 Geolocation");

        auto options = "Manual;IP Address (ipapi.co);WiFi Stations (Google)";
        if (b.Select(db::geo_method, "Method", options))
        {
            requestNewData();
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

    int GeoLocation::getTimeZoneOff() const
    {
        int tzh, tzm;
        decodeTimeZone(tzh, tzm);
        return (tzh * 60 + tzm) * 60;
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

    void GeoLocation::requestNewData()
    {
        m_fetchTS = millis();
        m_fetchTS -= FETCH_PERIOD;
        m_fetchTS += RETRY_PERIOD;
        log_i("request new data!");
    }

    void GeoLocation::confirmDataReceived()
    {
        m_fetchTS = millis();
        log_i("confirm data received!");
    }

    bool GeoLocation::fetchNewData()
    {
        float lat, lon; int tzh, tzm;
        auto updateOnSuccess = [&](bool success)
        {
            if (success)
            {
                settings::data()[db::geo_latitude ] = lat;
                settings::data()[db::geo_longitude] = lon;
                encodeTimeZone(tzh, tzm);
            }
            return success;
        };

        switch (getMethod())
        {
            case Method::IPAddress: 
                return updateOnSuccess(fetchDataUsingIPAddress(lat, lon, tzh, tzm));
            case Method::WiFiStations:
                return updateOnSuccess(fetchDataUsingWiFiStations(lat, lon, tzh, tzm));
        }
        return true;
    }

    bool GeoLocation::fetchDataUsingIPAddress(float& lat, float& lon, int& tzh, int& tzm) 
    {
        HTTPClient http;
        if (http.begin("https://ipapi.co/json/")) 
        {
            if (http.GET() == HTTP_CODE_OK) 
            {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, http.getStream());
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

    bool GeoLocation::fetchDataUsingWiFiStations(float &lat, float &lon, int &tzh, int &tzm)
    {
        bool result = false;
        WiFi.scanNetworks();
        if (callGoogleGeolocationApi(lat, lon))
        {
            long timestamp = service::dateTime.getNow();
            result = callGoogleTimeZoneApi(lat, lon, timestamp, tzh, tzm);
        }
        WiFi.scanDelete();
        return false;
    }

    bool GeoLocation::callGoogleGeolocationApi(float &lat, float &lon)
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

        log_i("url:\n%s", url.c_str());
        log_i("payload:\n%s", payload);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        if (http.POST((uint8_t*)payload, payloadLen) == HTTP_CODE_OK) 
        {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, http.getStream());
            if (!error)
            {
                lat = doc["location"]["lat"].as<float>();
                lon = doc["location"]["lng"].as<float>();
                http.end();
                return true;
            }
            http.end();
        }
        return false;
    }

    bool GeoLocation::callGoogleTimeZoneApi(float lat, float lon, long timestamp, int &tzh, int &tzm)
    {
        auto key = settings::apikey(db::apikey_google);
        auto url = String("https://maps.googleapis.com/maps/api/timezone/json?location=")
            + String(lat, 6) + "," + String(lon, 6)
            + "&timestamp=" + String(timestamp)
            + "&key=" + key;

        log_i("url:\n%s", url.c_str());

        HTTPClient http;
        http.begin(url);
        if (http.GET() == HTTP_CODE_OK) 
        {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, http.getStream());
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
            http.end();
        }
        return false;
    }

    GeoLocation geoLocation;
}
