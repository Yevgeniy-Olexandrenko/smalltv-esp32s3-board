#pragma once

#include "webserver/SettingsProvider.h"

namespace service
{
    class NetworkConnectionClass : public webserver::SettingsProvider
    {
        enum class State { ConnectRequested, Connecting, Connected, NotConnected, ScanRequested, Scanning };

    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        void buildWiFiScanResult(sets::Builder& b, int max);
        void buildWiFiConnection(sets::Builder& b);

    private:
        State m_state = State::NotConnected;
        String m_ssid;
        String m_pass;
    };

    extern NetworkConnectionClass NetworkConnection;
}
