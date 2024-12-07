#pragma once

#include <SettingsBase.h>
#include "BackgroundService.h"

class WeatherServiceClass : public BackgroundService
{
public:
    void begin();
    void update();

    void settingsBuild(sets::Builder& b);
    void settingsUpdate(sets::Updater& u);
};

extern WeatherServiceClass WeatherService;
