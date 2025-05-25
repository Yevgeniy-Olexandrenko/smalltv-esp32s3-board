#include "GeoLocation.h"
#include "services/wifi_connection/WiFiConnection.h"
#include "settings.h"

#define FETCH_PERIOD (30 * 60 * 1000) // 30 minutes
#define RETRY_PERIOD (3 * 1000)       // 3 seconds

namespace service
{
    void GeoLocation::begin()
    {
        settings::data().init(db::geo_method, int(Method::FromIPAddress));
        settings::data().init(db::geo_latitude, 50.4500f);
        settings::data().init(db::geo_longitude, 30.5233f);
        settings::data().init(db::geo_timezone, +2 * 100);
        requestNewData();
    }

    void GeoLocation::update()
    {
        if (millis() - m_fetchTS >= FETCH_PERIOD)
        {
            bool hasInternet = service::wifiConnection.isInternetAccessible();
            if (hasInternet && hasNewData())
                confirmDataReceived();
            else
                requestNewData();
        }
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        b.beginGuest();
        sets::Group g(b, "📍 Geolocation");

        auto options = "Manual;From IP Address (ipapi.co);From WiFi Stations (Google)";
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

            int off = settings::data()[db::geo_timezone];
            int tzh = off / 100, tzm = off % 100;
            int num = (tzm == 0 ? tzh : tzh * 60 + tzm);
            if (b.Number("Time zone", &num))
            {
                if (num >= -14 && num <= +12)
                    tzh = num, tzm = 0;
                else
                    tzh = num / 60, tzm = num % 60;
                settings::data()[db::geo_timezone ] = tzh * 100 + tzm;
            }
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
        int off = settings::data()[db::geo_timezone];
        int tzh = off / 100, tzm = off % 100;
        return (tzh * 60 + tzm) * 60;
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

    bool GeoLocation::hasNewData()
    {
        float lat, lon; int tzh, tzm;
        auto updateOnSuccess = [&lat, &lon, &tzh, tzm](bool success)
        {
            if (success)
            {
                settings::data()[db::geo_latitude ] = lat;
                settings::data()[db::geo_longitude] = lon;
                settings::data()[db::geo_timezone ] = tzh * 100 + tzm;
            }
            return success;
        };

        switch (getMethod())
        {
            case Method::FromIPAddress: 
                return updateOnSuccess(m_fromIPAddress.request(lat, lon, tzh, tzm));
            case Method::FromWiFiStations:
                return updateOnSuccess(m_fromWiFiStations.request(lat, lon, tzh, tzm));
        }
        return true;
    }

    String GeoLocation::getTimeZone() const
    {
        int off = settings::data()[db::geo_timezone];
        int tzh = off / 100, tzm = off % 100;

        char buffer[6];
        if (tzm == 0)
            sprintf(buffer, "%+d", tzh);
        else
            sprintf(buffer, "%+d:%02d", tzh, tzm);
        return String(buffer);
    }

    GeoLocation geoLocation;
}
