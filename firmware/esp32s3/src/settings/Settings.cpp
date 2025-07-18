#include "Settings.h"

bool Settings::m_init = false;
bool Settings::m_apiKeys = false;
GyverDBFile Settings::m_dbApiKeys;
GyverDBFile Settings::m_dbSettings;
SettingsGyver Settings::m_webServer;

void Settings::init()
{
    if (!m_init)
    {
        LittleFS.begin(true);
        m_dbApiKeys.setFS(&LittleFS, "/apikeys.db");
        m_dbSettings.setFS(&LittleFS, "/settings.db");

        m_dbApiKeys.begin();
        m_dbSettings.begin();
        m_webServer.begin();

        endApiKeys();
        m_init = true;
    }
}

GyverDBFile& Settings::data()
{
    return (m_apiKeys ? m_dbApiKeys : m_dbSettings);
}

SettingsGyver& Settings::sets()
{
    return m_webServer;
}

void Settings::beginApiKeys()
{
    m_webServer.attachDB(&m_dbApiKeys);
    m_apiKeys = true;
}

void Settings::endApiKeys()
{
    m_webServer.attachDB(&m_dbSettings);
    m_apiKeys = false;
}

const String Settings::apikey(size_t hash)
{
    beginApiKeys();
    String key = data()[hash];
    endApiKeys();
    return key;
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
