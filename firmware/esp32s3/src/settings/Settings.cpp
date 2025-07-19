#include "Settings.h"
#include "drivers/Storage.h"
#include "firmware/defines.h"

bool Settings::m_initDBs = false;
bool Settings::m_initSets = false;

GyverDBFile Settings::m_dbKeys;
GyverDBFile Settings::m_dbData;
SettingsGyver Settings::m_sets;

void Settings::initData()
{
    if (!m_initDBs)
    {
        auto flashFS = &driver::storage.getFlashFS();
        m_dbKeys.setFS(flashFS, "/settings_keys.db");
        m_dbData.setFS(flashFS, "/settings_data.db");
        m_dbKeys.begin();
        m_dbData.begin();
        m_initDBs = true;
    }
}

void Settings::initSets()
{
    if (!m_initSets)
    {
        m_sets.begin(true, getHostName().c_str());
        m_sets.fs.setFS(driver::storage.getFlashFS());
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
    sets().tick();
    keys().tick();
}

const String Settings::getHostName()
{
    return (WIFI_HOST_NAME + getDeviceID());
}

const String Settings::getDeviceID()
{
    String mac = WiFi.macAddress();
    return mac.substring(12, 14) + mac.substring(15);
}

const String Settings::getUserAgent()
{
    return (getHostName() + ("/" PROJECT_VERSION " (contact: " PROJECT_AUTHOR ")"));
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
