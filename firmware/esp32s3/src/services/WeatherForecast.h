#pragma once

#include "settings/Settings.h"

namespace service
{
    class WeatherForecast : public Settings::Provider
    {
    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;
    };

    extern WeatherForecast weatherForecast;
}
