#pragma once

#include <SettingsBase.h>
#include "BackgroundService.h"

class DateAndTimeServiceClass : public BackgroundService
{
public:
    void begin();
    void update();

    void settingsBuild(sets::Builder& b);
    void settingsUpdate(sets::Updater& u);
};

extern DateAndTimeServiceClass DateAndTimeService;
