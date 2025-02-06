#pragma once

#include "shared/settings/Settings.h"

namespace service_settings_webapp_impl
{
    class Main : public settings::Provider
    {
    public:
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        String getHTML() const;
        String getUptime() const;
        String getESPModuleInfo() const;
        String getRAMUsageInfo() const;
        String getPSRAMUsageInfo() const;
        String getPowerSourceInfo() const;
        String getWiFiSignalInfo() const;
    };
}
