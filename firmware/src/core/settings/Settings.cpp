#include "Settings.h"

namespace settings
{
    // only internal storage is supported
    static GyverDBFile m_database(&LittleFS, "/settings.db");
    static SettingsGyver m_settings("", &m_database);
    static bool m_databaseReady = false;
    static bool m_settingsReady = false;

    GyverDBFile& data()
    {
        if (!m_databaseReady)
        {
            log_i("begin database");
            m_databaseReady = true;
            LittleFS.begin(true);
            m_database.begin();
        }
        return m_database;
    }

    SettingsGyver& sets()
    {
        if (!m_settingsReady)
        {
            log_i("begin settings");
            m_settingsReady = true;
            m_settings.begin();
        }
        return m_settings;
    }

    void apikey(size_t hash, const String& def)
    { 
        data().init(hash, def);
    }

    const String apikey(size_t hash)
    { 
        return data()[hash];
    }

    const String Provider::led(Led led) const
    {
        switch(led)
        {
            case Led::R: return "🔴";
            case Led::Y: return "🟡";
            case Led::G: return "🟢";
        }
        return "";
    }
}
