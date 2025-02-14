#include <ip_loc.hpp>
#include "GeoLocation.h"
#include "../network_connection/NetworkConnection.h"

#define UPDATE_PERIOD (60 * 1000)

namespace service
{
    void GeoLocation::begin()
    {
        m_fetchTS = 0;
        m_lat = 48.3794f;
        m_lon = 31.1656f;
        m_utcOffset = 0;
        m_region[0] = 0;
        m_city[0] = 0;
        m_tz[0] = 0;
    }

    void GeoLocation::update()
    {
        return;

        auto now = millis();
        if (now - m_fetchTS >= UPDATE_PERIOD)
        {
            if (service::networkConnection.isInternetAccessible())
            {
                bool ok = arduino::ip_loc::fetch(
                    &m_lat, &m_lon, &m_utcOffset,
                    m_region, sizeof(m_region),
                    m_city, sizeof(m_city),
                    m_tz, sizeof(m_tz)
                );

                if (ok) 
                {
                    log_i("--------------------");
                    log_i("lat: %f", m_lat);
                    log_i("lon: %f", m_lon);
                    log_i("utc_offset: %d", m_utcOffset);
                    log_i("region: %s", m_region);
                    log_i("city: %s", m_city);
                    log_i("tz: %s", m_tz);
                }

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

                if (ok || m_location.status)
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