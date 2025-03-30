#pragma once

#include "shared/tasks/Task.h"
#include "WiFiConnectionUI.h"

namespace service
{
    using namespace wifi_connection;

    class WiFiConnection
        : public task::Task<1024, task::core::System, task::priority::Background>
    {
        enum class State { ConnectRequested, Connecting, Connected, NotConnected, ScanRequested, Scanning };

    public:
        enum class Signal { Excellent, Good, Fair, Bad };

        void begin();
        void update();


        int getSignalRSSI() const;
        Signal getSignalStrength() const;

        bool isInAccessPointMode() const;
        bool isInternetAccessible() const;

        WiFiConnectionUI& getUI() { return m_ui; }

    private:
        void task() override;

        void beginConnection(const String& ssid, const String& pass, uint8_t tout);
        void updateConnection();
        bool isConnected() const;
        void startStationAndAccessPoint(const char* ssid, const char* pass);
        void startAccessPoint();
        void stopAccessPoint();

    private:
        bool m_conTrying = false;
        uint32_t m_conStartTS;
        uint32_t m_conTimeout;

        State m_state = State::NotConnected;
        WiFiConnectionUI m_ui;
    };

    extern WiFiConnection wifiConnection;
}
