#pragma once

#include "shared/settings/Settings.h"

namespace service_settings_webapp_impl
{
    class Apps : public settings::Provider
    {
    public:
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    };
}
