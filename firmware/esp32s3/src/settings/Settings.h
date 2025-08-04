#pragma once

#include <GyverDBFile.h>
#include <SettingsESP.h>
#include "DBKeys.h"
#include <WebDAV.h>

class Settings
{
    static void initData();
    static void initSets();

public:
    static GyverDBFile& keys();
    static GyverDBFile& data();
    static SettingsESP& sets();

    static void tick();

    static const String getHostName();
    static const String getDeviceID();
    static const String getUserAgent();

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
    static bool m_initData;
    static bool m_initSets;
    static GyverDBFile m_keys;
    static GyverDBFile m_data;
    static SettingsESP m_sets;
    static WebDAVHandler m_webdav;
};
