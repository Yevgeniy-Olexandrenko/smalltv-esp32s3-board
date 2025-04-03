#pragma once

#include "shared/settings/Settings.h"

namespace service::wifi_connection
{
    class WiFiConnectionUI : public settings::Provider
    {
    public:
        void begin();
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        bool isManualInput() const;
        bool isClosedNetwork() const;
        void fetchAvailableAPOptions(String& options, std::vector<String>& values);

    private:
        String m_ssid;
        String m_pass;

        bool m_gotoManualReq;
        bool m_scanRequested;
        bool m_connRequested;
    };
}