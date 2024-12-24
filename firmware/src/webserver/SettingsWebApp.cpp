#include "SettingsWebApp.h"
#include "firmware.h"

#include "LittleFS.h"

#include "services/NetworkConnection.h"
#include "services/GeoLocation.h"
#include "services/DateAndTime.h"
#include "services/Weather.h"

#include "HardwareInfo.h"

namespace webserver
{
    SettingsClass::SettingsClass()
        // only internal storage is supported
        : m_database(&LittleFS, "/settings.db")
        , m_settings(SETTINGS_TITLE, &m_database)
        , m_databaseReady(false)
        , m_settingsReady(false)
    {
    }

    GyverDBFile &SettingsClass::data()
    {
        if (!m_databaseReady)
        {
            Serial.println("Settings: begin database");
            m_databaseReady = true;
            LittleFS.begin();
            m_database.begin();
        }
        return m_database;
    }

    SettingsGyver &SettingsClass::sets()
    {
        if (!m_settingsReady)
        {
            Serial.println("Settings: begin settings");
            m_settingsReady = true;
            m_settings.begin();
        }
        return m_settings;
    }

    void SettingsWebAppClass::begin()
    {
        Serial.println("SettingsWebAppClass: begin");
        Settings.sets().onBuild([&](sets::Builder& b) { this->settingsBuild(b); });
        Settings.sets().onUpdate([&](sets::Updater& u) { this->settingsUpdate(u); });
        m_restartRequested = false;
    }

    void SettingsWebAppClass::update()
    {
        Settings.sets().tick();
        if (m_restartRequested) ESP.restart();
    }

    void SettingsWebAppClass::settingsBuild(sets::Builder& b) 
    {
        service::networkConnection.settingsBuild(b);
        {
            sets::Group g(b, "Settings");
            {
                sets::Menu m(b, "Globals");
                service::GeoLocation.settingsBuild(b);
                service::DateAndTime.settingsBuild(b);
                service::Weather.settingsBuild(b);
            }
            {
                sets::Menu m(b, "Applications");
                b.Label("TODO");
            }
        }
        HardwareInfo.settingsBuild(b);

        // TODO

        if (b.Button("Restart"))
        {
            m_restartRequested = true;
            Settings.data().update();
            b.reload();
        }
    }
        
    void SettingsWebAppClass::settingsUpdate(sets::Updater& u)
    {
        // handle services
        service::networkConnection.settingsUpdate(u);
        service::GeoLocation.settingsUpdate(u);
        service::DateAndTime.settingsUpdate(u);
        service::Weather.settingsUpdate(u);

        HardwareInfo.settingsUpdate(u);

        // TODO
    }

    SettingsClass Settings;
    SettingsWebAppClass SettingsWebApp;
}
