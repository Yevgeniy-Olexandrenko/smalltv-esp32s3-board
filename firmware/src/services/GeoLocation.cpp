#include "GeoLocation.h"
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
            m_request = true;
            b.reload();
        }
        else
        {
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
                b.Label("coords"_h, "Coordinates", getCoordinates());
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
            u.update("coords"_h, getCoordinates());
            u.update("timezone"_h, getTimeZone());
        }
    }

    void GeoLocation::task()
    {
        while (!requestGeoLocation()) sleep(RETRY_PERIOD_MS);
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

    String GeoLocation::getCoordinates() const
    {
        return String(getLatitude(), 4) + ", " + String(getLongitude(), 4);
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
                ok = m_requests.requestUsingIPAddress(lat, lon, tzh, tzm);
                break;

            case Method::WiFiStations:
                ok = m_requests.requestUsingWiFiStations(lat, lon, tzh, tzm);
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

    GeoLocation geoLocation;
}
