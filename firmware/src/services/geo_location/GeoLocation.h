#pragma once

#include "shared/settings/Settings.h"

namespace service
{
    class GeoLocation : public settings::Provider
    {
    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;
    };

    extern GeoLocation geoLocation;
}
