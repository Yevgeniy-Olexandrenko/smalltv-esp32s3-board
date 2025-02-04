#pragma once

#include "shared/settings/Settings.h"

namespace service
{
    class NetworkConnection : public settings::Provider
    {
        enum class State { ConnectRequested, Connecting, Connected, NotConnected, ScanRequested, Scanning };

    public:
        enum class Signal { Excellent, Good, Fair, Bad };

        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

        int getSignalRSSI() const;
        Signal getSignalStrength() const;

    private:
        void buildWiFiScanResult(sets::Builder& b, int max);
        void buildWiFiConnection(sets::Builder& b);

    private:
        State  m_state = State::NotConnected;
        String m_ssid;
        String m_pass;
    };

    extern NetworkConnection networkConnection;
}
