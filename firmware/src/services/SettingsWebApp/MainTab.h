#pragma once

#include "core/settings/Settings.h"

namespace service::details
{
    class MainTab : public settings::Provider
    {
    public:
        void begin() {}
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        void briefInfoBuild(sets::Builder& b);
        void briefInfoUpdate(sets::Updater& u);
        void audioPlayerBuild(sets::Builder& b);
        void audioPlayerUpdate(sets::Updater& u);
        void hardwareInfoBuild(sets::Builder& b);
        void hardwareInfoUpdate(sets::Updater& u);

        String getHTML() const;
        String getInet() const;
        String getUptime() const;
        String getESPModuleInfo() const;
        String getRAMUsageInfo() const;
        String getPSRAMUsageInfo() const;
        String getPowerSourceInfo() const;
        String getWiFiSignalInfo() const;
    };
}
