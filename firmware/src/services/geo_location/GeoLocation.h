#pragma once

#include <GeoIP.h>
#include "core/settings/Settings.h"
#include "FromIPAddress.h"
#include "FromWiFiStations.h"

namespace service
{
    class GeoLocation : public settings::Provider
    {
    public:
        enum class Method { Manual, FromIPAddress, FromWiFiStations };

        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

        Method getMethod() const;
        float  getLatitude() const;
        float  getLongitude() const;

    private:
        geo_location::FromIPAddress m_fromIPAddress;
        geo_location::FromWiFiStations m_fromWiFiStations;
    };

    extern GeoLocation geoLocation;
}
