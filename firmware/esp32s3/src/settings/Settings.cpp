#include "Settings.h"

bool Settings::m_initDBs = false;
bool Settings::m_initSets = false;

GyverDBFile Settings::m_dbKeys;
GyverDBFile Settings::m_dbData;
SettingsGyver Settings::m_sets;

void Settings::initData()
{
    if (!m_initDBs)
    {
        LittleFS.begin(true);
        m_dbKeys.setFS(&LittleFS, "/settings_keys.db");
        m_dbData.setFS(&LittleFS, "/settings_data.db");
        m_dbKeys.begin();
        m_dbData.begin();
        m_initDBs = true;
    }
}

void Settings::initSets()
{
    if (!m_initSets)
    {
        m_sets.begin();
        m_sets.attachDB(&m_dbData);
        m_initSets = true;
    }
}

GyverDBFile& Settings::data()
{
    initData();
    return m_dbData;
}

GyverDBFile& Settings::keys()
{
    initData();
    return m_dbKeys;
}

SettingsGyver& Settings::sets()
{
    initSets();
    return m_sets;
}

void Settings::tick()
{
    keys().tick();
    data().tick();
    sets().tick();
}

const String Settings::Provider::led(Led led) const
{
    switch(led)
    {
        case Led::R: return "🔴";
        case Led::Y: return "🟡";
        case Led::G: return "🟢";
    }
    return "";
}
