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
        void fillESPModuleInfo(String& moduleSpecs);
        void fillRAMUsageInfo(String& ramUsage);
        void fillPSRAMUsageInfo(String& psramUsage);
        void fillPowerSourceInfo(String& powerSource);
        void fillWiFiSignalInfo(String& wifiSignal);
    };
}
