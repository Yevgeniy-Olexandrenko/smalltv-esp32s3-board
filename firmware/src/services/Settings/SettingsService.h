#pragma once

#include <GyverDBFile.h>
#include <SettingsGyver.h>
#include "services/Service.h"

class SettingsServiceClass : public Service
{
public:
    SettingsServiceClass();
    void update();

    GyverDBFile& data();
    SettingsGyver& sets();

private:
    GyverDBFile m_database;
    SettingsGyver m_settings;
    bool m_databaseReady;
    bool m_settingsReady;
};

extern SettingsServiceClass SettingsService;
