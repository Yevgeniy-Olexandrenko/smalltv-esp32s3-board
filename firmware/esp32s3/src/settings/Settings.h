#pragma once

#include <GyverDBFile.h>
#include <SettingsGyver.h>
#include "DBKeys.h"

class Settings
{
    static void init();

public:
    static GyverDBFile& data();
    static SettingsGyver& sets();

    static void beginApiKeys();
    static void endApiKeys();

    static const String apikey(size_t hash);

public:
    class Provider
    {
    public:
        virtual void settingsBuild(sets::Builder& b) = 0;
        virtual void settingsUpdate(sets::Updater& u) = 0;

    protected:
        enum class Led { R, Y, G };
        const String led(Led led) const;
    };

private:
    static bool m_init;
    static bool m_apiKeys;
    static GyverDBFile m_dbApiKeys;
    static GyverDBFile m_dbSettings;
    static SettingsGyver m_webServer;
};
