#pragma once

#include "core/settings/Settings.h"

namespace service::settings_webapp
{
    class Apps : public settings::Provider
    {
    public:
        void begin() {}
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    };
}
