#pragma once

#include "core/settings/Settings.h"

namespace service::wifi_connection
{
    class WiFiConnectionUI : public settings::Provider
    {
        enum class Action { None, DoScan, DoConnect, GoToManual };

    public:
        void begin();
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        bool isManualInput() const;
        bool isPassClosedNetwork() const;
        void fillSSIDsOptions(String& options);
        void chooseSSIDByIndex(size_t index);

    private:
        String m_ssid, m_pass;
        Action m_action{ Action::None };
    };
}
