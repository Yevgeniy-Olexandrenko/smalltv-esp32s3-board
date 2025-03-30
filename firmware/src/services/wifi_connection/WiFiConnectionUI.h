#pragma once

#include "shared/settings/Settings.h"

namespace service::wifi_connection
{
    class WiFiConnectionUI : public settings::Provider
    {
    public:
        void begin();

        const String& getSSID() const { return m_ssid; }
        const String& getPass() const { return m_pass; }

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        bool isScanning() const;
        bool isConnecting() const;
        bool isManualInput() const;
        void fetchWiFiStationsOptions(String& options, std::vector<String>& values);

    private:
        String m_ssid;
        String m_pass;
    };
}