#pragma once

#include <GyverDBFile.h>
#include <SettingsGyver.h>

namespace webserver
{
    class SettingsClass
    {
    public:
        SettingsClass();
        GyverDBFile& data();
        SettingsGyver& sets();

    private:
        GyverDBFile m_database;
        SettingsGyver m_settings;
        bool m_databaseReady;
        bool m_settingsReady;
    };

    class SettingsWebAppClass
    {
    public:
        void begin();
        void update();

    private:
        void settingsBuild(sets::Builder& b);
        void settingsUpdate(sets::Updater& u);

    private:
        bool m_restartRequested;
    };

    extern SettingsClass Settings;
    extern SettingsWebAppClass SettingsWebApp;
}
