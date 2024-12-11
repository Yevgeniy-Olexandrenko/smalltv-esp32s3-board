#pragma once

#include "services/Service.h"
#include "services/Settings/SettingsProvider.h"

class DateAndTimeServiceClass : public Service, public SettingsProvider
{
public:
    void begin();
    void update();

    void settingsBuild(sets::Builder& b);
    void settingsUpdate(sets::Updater& u);
};

extern DateAndTimeServiceClass DateAndTimeService;
