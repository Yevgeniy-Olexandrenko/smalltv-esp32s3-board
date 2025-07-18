#pragma once

#include "core/Core.h"
#include "WiFiConnection/WiFiConnectionUI.h"
#include "WiFiConnection/InternetAccess.h"

namespace service
{
    class WiFiConnection
        : public core::Task<4096, core::TaskCpu::System, core::TaskPrio::Background>
    {
    public:
        enum class Signal { Excellent, Good, Fair, Bad };

        void begin();
        void connect(const String& ssid, const String& pass);

        bool isConnecting() const { return m_connect.trying; }
        bool isConnectedToAP() const { return WiFi.isConnected(); }
        bool isInAccessPointMode() const { return !isConnectedToAP() && WiFi.getMode() == WIFI_MODE_AP; }
        bool isInternetAvailable() const { return m_internet.available(); }
    
        int8_t getRSSI() const { return WiFi.RSSI(); }
        Signal getSignal(int8_t rssi) const;
        String getDeviceID() const;
        String getUserAgent() const;

        details::WiFiConnectionUI& getUI() { return m_ui; }

    private:
        void task() override;
        void beginConnection();
        void updateConnection();
        void handleEvent(WiFiEvent_t event, WiFiEventInfo_t info);

    private:
        details::WiFiConnectionUI m_ui;
        struct {
            String ssid;
            String pass;
            uint32_t tout = 0;
            uint32_t time = 0;
            bool trying = false;
        } m_connect; 
        details::InternetAccess m_internet;
    };

    extern WiFiConnection wifiConnection;
}
