#pragma once

#include "settings/Settings.h"

namespace service::details
{
    class AppsTab : public Settings::Provider
    {
    public:
        void begin() {}
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    };
}
