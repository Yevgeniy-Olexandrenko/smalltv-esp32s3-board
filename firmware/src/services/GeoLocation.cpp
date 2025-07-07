#include "GeoLocation.h"
#include "services/WiFiConnection.h"
#include "GeoLocation/GeoLocationRequests.h"
#include "settings.h"

namespace service
{
    void GeoLocation::begin()
    {
        settings::data().init(db::geo_method, int(Method::IPAddress));
        settings::data().init(db::geo_latitude,  50.4500f);
        settings::data().init(db::geo_longitude, 30.5233f);
        settings::data().init(db::geo_timezone,  200);
        decodeCoords(m_lat, m_lon);

        m_request = false;
        startRequest();
    }

    void GeoLocation::startRequest()
    {
        Task::stopRepeat();
        Task::stop();

        Task::start("geo_location");
        Task::startRepeat("geo_location", RESTART_PERIOD_MS);
    }

    GeoLocation::Method GeoLocation::getMethod() const
    {
        return Method(int(settings::data()[db::geo_method]));
    }

    int GeoLocation::getTZOffset() const
    {
        int tzh, tzm;
        decodeTimeZone(tzh, tzm);
        return (tzh * 60 + tzm) * 60;
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        b.beginGuest();
        sets::Group g(b, "📍 Geolocation");

        auto options = "Manual;IP Address (ipapi.co);WiFi Stations (Google)";
        if (b.Select(db::geo_method, "Method", options))
        {
            m_request = true;
            b.reload();
        }
        else
        {
            if (getMethod() == Method::Manual)
            {
                if (b.Number("Latitude", &m_lat, -90.f, +90.f))
                {
                    m_request = true;
                }
                if (b.Number("Longitude", &m_lon, -180.f, +180.f))
                {
                    m_request = true;
                }

                int tzh, tzm;
                decodeTimeZone(tzh, tzm);
                int num = (tzm == 0 ? tzh : tzh * 60 + tzm);
                if (b.Number("Time zone", &num)) setTimeZone(num);
            }
            else
            {
                b.Label("coords"_h, "Coordinates", getCoords());
                b.LabelNum("timezone"_h, "Time zone", getTimeZone());
            }
        }
        b.endGuest();
    }

    void GeoLocation::settingsUpdate(sets::Updater& u)
    {
        if (m_request)
        {
            startRequest();
            m_request = false;
        }
        if (getMethod() != Method::Manual)
        {
            u.update("coords"_h, getCoords());
            u.update("timezone"_h, getTimeZone());
        }
    }

    void GeoLocation::task()
    {
        while (!requestGeoLocation()) sleep(RETRY_PERIOD_MS);
        settings::sets().reload();
    }

    void GeoLocation::decodeCoords(float& lat, float& lon)
    {
        lat = settings::data()[db::geo_latitude ];
        lon = settings::data()[db::geo_longitude];
    }

    void GeoLocation::encodeCoords(float& lat, float& lon)
    {
        settings::data()[db::geo_latitude ] = lat;
        settings::data()[db::geo_longitude] = lon;
    }

    void GeoLocation::decodeTimeZone(int& tzh, int& tzm)
    {
        int off = settings::data()[db::geo_timezone];
        tzh = off / 100, tzm = abs(off) % 100;
    }
    
    void GeoLocation::encodeTimeZone(int& tzh, int& tzm)
    {
        int off = (tzh * 100 + tzm);
        settings::data()[db::geo_timezone] = off;
    }

    void GeoLocation::generateCountryFlag(const String& countryCode, String& countryFlag)
    {
        countryFlag = countryCode;
        if (countryCode.length() != 2 || 
            countryCode[0] < 'A' || countryCode[0] > 'Z' || 
            countryCode[1] < 'A' || countryCode[1] > 'Z') return;

        auto encodeUtf8 = [](uint32_t codePoint, char* output) 
        {
            if (codePoint <= 0x7F) 
            {
                output[0] = codePoint;
                return 1;
            } 
            else if (codePoint <= 0x7FF) 
            {
                output[0] = 0xC0 | (codePoint >> 6);
                output[1] = 0x80 | (codePoint & 0x3F);
                return 2;
            } 
            else if (codePoint <= 0xFFFF) 
            {
                output[0] = 0xE0 | (codePoint >> 12);
                output[1] = 0x80 | ((codePoint >> 6) & 0x3F);
                output[2] = 0x80 | (codePoint & 0x3F);
                return 3;
            } 
            else if (codePoint <= 0x10FFFF) 
            {
                output[0] = 0xF0 | (codePoint >> 18);
                output[1] = 0x80 | ((codePoint >> 12) & 0x3F);
                output[2] = 0x80 | ((codePoint >> 6) & 0x3F);
                output[3] = 0x80 | (codePoint & 0x3F);
                return 4;
            }
            return 0;
        };

        uint32_t codePoint1 = 0x1F1E6 + (countryCode[0] - 'A');
        uint32_t codePoint2 = 0x1F1E6 + (countryCode[1] - 'A');

        char utf8flag[9]; int len = 0;
        len += encodeUtf8(codePoint1, utf8flag + len);
        len += encodeUtf8(codePoint2, utf8flag + len);
        utf8flag[len] = '\0';

        countryFlag = utf8flag;
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

    String GeoLocation::getTimeZone() const
    {
        int tzh, tzm;
        decodeTimeZone(tzh, tzm);
        char buffer[6];
        if (tzm == 0) sprintf(buffer, "%+d", tzh);
        else sprintf(buffer, "%+d:%02d", tzh, tzm);
        return String(buffer);
    }

    String GeoLocation::getCoords() const
    {
        return String(getLatitude(), 4) + ", " + String(getLongitude(), 4);
    }

    bool GeoLocation::isNewCoords() const
    {
        float lat, lon;
        decodeCoords(lat, lon);
        return (m_lat != lat || m_lon != lon);
    }

    bool GeoLocation::requestGeoLocation()
    {
        log_i("request geolocation");

        int tzh, tzm;
        bool ok = false;
        switch (getMethod())
        {
            case Method::Manual:
                decodeTimeZone(tzh, tzm);
                ok = service::wifiConnection.isInternetAccessible();
                break;

            case Method::IPAddress: 
                ok = m_requests.requestUsingIPAddress(m_lat, m_lon, tzh, tzm);
                break;

            case Method::WiFiStations:
                ok = m_requests.requestUsingWiFiStations(m_lat, m_lon, tzh, tzm);
                break;
        }
        if (ok)
        {
            if (!hasLocality() || isNewCoords())
            {
                log_i("request locality: %f,%f", m_lat, m_lon);
                if (m_requests.requestReverseGeocoding(m_lat, m_lon, m_locality, m_countryCode))
                {
                    generateCountryFlag(m_countryCode, m_countryFlag);
                }
            }
            encodeCoords(m_lat, m_lon);
            encodeTimeZone(tzh, tzm);
        }
        log_i("request geolocation: %s", (ok ? "SUCCESS" : "FAILED"));
        return ok;
    }

    GeoLocation geoLocation;
}
