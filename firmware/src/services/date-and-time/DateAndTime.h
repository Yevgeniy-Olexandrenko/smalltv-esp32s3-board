#pragma once

#include "shared/settings/Settings.h"

namespace service
{
    class DateAndTime : public settings::Provider
    {
    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b);
        void settingsUpdate(sets::Updater& u);
    };
    
    extern DateAndTime dateAndTime;
}
