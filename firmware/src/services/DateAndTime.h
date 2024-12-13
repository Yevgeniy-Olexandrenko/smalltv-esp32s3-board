#pragma once

#include "webserver/SettingsProvider.h"

namespace service
{
    class DateAndTimeClass : public webserver::SettingsProvider
    {
    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b);
        void settingsUpdate(sets::Updater& u);
    };
    
    extern DateAndTimeClass DateAndTime;
}
