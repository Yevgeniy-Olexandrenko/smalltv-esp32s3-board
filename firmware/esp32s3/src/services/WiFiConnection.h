#pragma once

#include "core/Core.h"
#include "WiFiConnection/WiFiConnectionUI.h"

namespace service
{
    class WiFiConnection
        : public core::Task<4096, core::TaskCpu::System, core::TaskPrio::Background>
    {
        constexpr static auto INET_CHECK_PERIOD_SEC    = 20;
        constexpr static auto INET_CHECK_TIMEOUT_SEC   = 2;
        constexpr static auto DEFAULT_CONNECT_TOUT_SEC = 20;

    public:
        enum class Signal { Excellent, Good, Fair, Bad };

        void begin();
        void connect(const String& ssid, const String& pass);

        bool isConnecting() const { return m_connect.trying; }
        bool isConnectedToAP() const { return WiFi.isConnected(); }
        bool isInAccessPointMode() const { return bool(WiFi.getMode() & WIFI_AP); }
        bool isInternetAvailable() const { return m_internet.available; }
    
        int8_t getRSSI() const { return WiFi.RSSI(); }
        Signal getSignal(int8_t rssi) const;
        
        details::WiFiConnectionUI& getUI() { return m_ui; }

    private:
        void task() override;
        void beginConnection();
        void updateConnection();
        void updateInternet();

    private:
        details::WiFiConnectionUI m_ui;
        struct {
            bool trying;
            String ssid, pass;
            core::TimerSec<> timer;
        } m_connect; 
        struct {
            bool available = false;
            core::TimerSec<INET_CHECK_PERIOD_SEC> timer;
        } m_internet;
    };

    extern WiFiConnection wifiConnection;
}
