#pragma once

#include "services/Service.h"
#include "services/Settings/SettingsProvider.h"

class GeoLocationServiceClass : public Service, public SettingsProvider
{
public:
    void begin();
    void update();

    void settingsBuild(sets::Builder& b) override;
    void settingsUpdate(sets::Updater& u) override;
};

extern GeoLocationServiceClass GeoLocationService;