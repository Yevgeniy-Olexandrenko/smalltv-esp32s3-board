#pragma once

#include <GeoIP.h>
#include "core/settings/Settings.h"

namespace service
{
    class GeoLocation : public settings::Provider
    {
        using Timestamp = unsigned long;

    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        Timestamp m_fetchTS;
        GeoIP m_geoip;
        location_t m_location;
    };

    extern GeoLocation geoLocation;
}
