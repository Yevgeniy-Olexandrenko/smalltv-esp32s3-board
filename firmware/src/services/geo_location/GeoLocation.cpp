#include "GeoLocation.h"
#include "services/wifi_connection/WiFiConnection.h"
#include "settings.h"

namespace service
{
    void GeoLocation::begin()
    {
        settings::data().init(db::geo_method, int(Method::FromIPAddress));
        settings::data().init(db::geo_latitude, 50.4500f);
        settings::data().init(db::geo_longitude, 30.5233f); 
    }

    void GeoLocation::update()
    {
        // TODO
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "📍 Geolocation");

        auto options = "Manual;From IP Address (ipapi.co);From WiFi Stations (Google)";
        if (b.Select(db::geo_method, "Method", options))
        {
            // TODO: on method change

            b.reload();
            return;
        }
        if (getMethod() == Method::Manual)
        {
            b.Number(db::geo_latitude, "Latitude", nullptr, -90.f, +90.f);
            b.Number(db::geo_longitude, "Longitude", nullptr, -180.f, +180.f);
        }
        else
        {
            b.LabelFloat("Latitude", getLatitude(), 4);
            b.LabelFloat("Longitude", getLongitude(), 4);
        }
    }

    void GeoLocation::settingsUpdate(sets::Updater &u)
    {
        // TODO
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

    GeoLocation geoLocation;
}