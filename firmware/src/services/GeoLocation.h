#pragma once

#include "webserver/SettingsProvider.h"

namespace service
{
    class GeoLocationClass : public webserver::SettingsProvider
    {
    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;
    };

    extern GeoLocationClass GeoLocation;
}
