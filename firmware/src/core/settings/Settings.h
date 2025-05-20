#pragma once

#include <GyverDBFile.h>
#include <SettingsGyver.h>

namespace settings
{
    GyverDBFile& data();
    SettingsGyver& sets();

    class Provider
    {
    public:
        virtual void settingsBuild(sets::Builder& b) = 0;
        virtual void settingsUpdate(sets::Updater& u) = 0;

    protected:
        enum class Led { R, Y, G };
        const String led(Led led) const;
    };
}
