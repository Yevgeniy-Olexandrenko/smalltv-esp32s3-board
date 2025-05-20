#include "GeoLocation.h"
#include "services/wifi_connection/WiFiConnection.h"

namespace service
{
    void GeoLocation::begin()
    {
        // TODO
    }

    void GeoLocation::update()
    {
        // TODO
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "📍 Geolocation");

        auto options = "Manual;By IP Address (ipapi.co);By WiFi Stations (Google)";
        auto selection = int(m_method);

        if (b.Select("Method", options, &selection))
        {
            SetMethod(Method(selection));
            b.reload();
            return;
        }
        if (m_method == Method::Manual)
        {
            b.Number("Latitude", &m_latitude, -90.f, +90.f);
            b.Number("Longitude", &m_longitude, -180.f, +180.f);
        }
        else
        {
            b.LabelFloat("Latitude", m_latitude, 4);
            b.LabelFloat("Longitude", m_longitude, 4);
        }
    }

    void GeoLocation::settingsUpdate(sets::Updater &u)
    {
        // TODO
    }

    void GeoLocation::SetMethod(Method method)
    {
        if (method != m_method)
        {
            m_method = method;

            // TODO
        }
    }

    GeoLocation geoLocation;
}