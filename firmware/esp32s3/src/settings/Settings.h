#pragma once

#include <GyverDBFile.h>
#include <SettingsGyver.h>
#include "DBKeys.h"

class Settings
{
    static void initData();
    static void initSets();

public:
    static GyverDBFile& keys();
    static GyverDBFile& data();

    static SettingsGyver& sets();
    static void tick();

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
    static bool m_initDBs;
    static bool m_initSets;
    static GyverDBFile m_dbKeys;
    static GyverDBFile m_dbData;
    static SettingsGyver m_sets;
};
