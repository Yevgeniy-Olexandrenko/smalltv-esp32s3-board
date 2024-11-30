#pragma once

#include <GyverDBFile.h>
#include <SettingsGyver.h>
#include "BackgroundService.h"

class SettingsServiceClass : public BackgroundService
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
