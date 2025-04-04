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
        bool isAuthClosedNetwork() const;
        void fetchSSIDScanResultOptions(String& options);
        void setSSIDFromScanResult(size_t index);

    private:
        String m_ssid;
        String m_pass;
        bool m_reqScan;
        bool m_reqConnect;
        bool m_reqGoToManual;
    };
}