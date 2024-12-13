#pragma once

#include "webserver/SettingsProvider.h"

namespace service
{
    class WeatherClass : public webserver::SettingsProvider
    {
    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;
    };

    extern WeatherClass Weather;
}
