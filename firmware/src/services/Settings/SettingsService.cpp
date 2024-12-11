#include "SettingsService.h"
#include "firmware.h"
#include <LittleFS.h>

SettingsServiceClass::SettingsServiceClass()
    : m_database(&LittleFS, "/settings.db")
    , m_settings(SETTINGS_TITLE, &m_database)
    , m_databaseReady(false)
    , m_settingsReady(false)
{
}

void SettingsServiceClass::update()
{
    m_settings.tick();
}

GyverDBFile &SettingsServiceClass::data()
{
    if (!m_databaseReady)
    {
        Serial.println("SettingsService: begin database");
        m_databaseReady = true;
        LittleFS.begin(true);
        m_database.begin();
    }
    return m_database;
}

SettingsGyver &SettingsServiceClass::sets()
{
    if (!m_settingsReady)
    {
        Serial.println("SettingsService: begin settings");
        m_settingsReady = true;
        m_settings.begin();
    }
    return m_settings;
}

SettingsServiceClass SettingsService;
