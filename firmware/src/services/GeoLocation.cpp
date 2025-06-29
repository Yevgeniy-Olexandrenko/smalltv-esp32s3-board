#include "GeoLocation.h"
#include "DateTime.h"
#include "settings.h"

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

    void GeoLocation::startRequest()
    {
        Task::stop();
        Task::start("geo_location");
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

    void GeoLocation::settingsBuild(sets::Builder& b)
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

    void GeoLocation::settingsUpdate(sets::Updater& u)
    {
        if (getMethod() != Method::Manual)
        {
            u.update("latitude"_h, getLatitude(), 4);
            u.update("longitude"_h, getLongitude(), 4);
            u.update("timezone"_h, getTimeZone());
        }
    }

    void GeoLocation::task()
    {
        while (!requestGeoLocation()) sleep(5000);
    }

    void GeoLocation::encodeTimeZone(int& tzh, int& tzm) const
    {
        int off = (tzh * 100 + tzm);
        settings::data()[db::geo_timezone ] = off;
    }

    void GeoLocation::decodeTimeZone(int& tzh, int& tzm) const
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
        if (m_http.begin("https://ipapi.co/json/")) 
        {
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

    bool GeoLocation::requestUsingWiFiStations(float& lat, float& lon, int& tzh, int& tzm)
    {
        bool ok = false;
        if (settings::apikey(db::apikey_google).length() >= 39)
        {
            if (WiFi.scanComplete() < 1) WiFi.scanNetworks();
            if (requestGoogleGeolocationApi(lat, lon))
            {
                long timestamp = service::dateTime.getNow();
                ok = requestGoogleTimeZoneApi(lat, lon, timestamp, tzh, tzm);
            }
            if (ok) WiFi.scanDelete();
        }
        return ok;
    }

    bool GeoLocation::requestGoogleGeolocationApi(float& lat, float& lon)
    {
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

        auto key = settings::apikey(db::apikey_google);
        auto url = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + key;

        m_http.begin(url);
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
        m_http.end();
        return false;
    }

    bool GeoLocation::requestGoogleTimeZoneApi(float lat, float lon, long timestamp, int& tzh, int& tzm)
    {
        auto key = settings::apikey(db::apikey_google);
        auto url = String("https://maps.googleapis.com/maps/api/timezone/json?location=")
            + String(lat, 6) + "," + String(lon, 6)
            + "&timestamp=" + String(timestamp)
            + "&key=" + key;

        m_http.begin(url);
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
        m_http.end();
        return false;
    }

    GeoLocation geoLocation;
}
