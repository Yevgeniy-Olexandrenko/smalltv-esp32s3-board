#include "GeoLocation.h"
#include "services/wifi_connection/WiFiConnection.h"

#define UPDATE_PERIOD (60 * 1000)

namespace service
{
    void GeoLocation::begin()
    {
        m_fetchTS = 0;
    }

    void GeoLocation::update()
    {
        return;

        auto now = millis();
        if (now - m_fetchTS >= UPDATE_PERIOD)
        {
            if (service::wifiConnection.isInternetAccessible())
            {
                m_location = m_geoip.getGeoFromWiFi();
                if (m_location.status)
                {
                    log_i("--------------------");
                    log_i("lat: %f", m_location.latitude);
                    log_i("lon: %f", m_location.longitude);
                    log_i("offset: %d", m_location.offset);
                    log_i("offset_sec: %d", m_location.offsetSeconds);
                    log_i("country: %s", m_location.country);
                    log_i("region: %s", m_location.region);
                    log_i("city: %s", m_location.city);
                    log_i("tz: %s", m_location.timezone);
                }

                if (m_location.status)
                {
                    m_fetchTS = now;
                }
            }
        }
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Geolocation");
        b.Label("TODO");
    }

    void GeoLocation::settingsUpdate(sets::Updater &u)
    {
        // TODO
    }

    GeoLocation geoLocation;
}