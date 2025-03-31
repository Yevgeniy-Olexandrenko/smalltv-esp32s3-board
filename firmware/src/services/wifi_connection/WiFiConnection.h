#pragma once

#include "shared/tasks/Task.h"
#include "WiFiConnectionUI.h"
#include "InternetAccess.h"

namespace service
{
    using namespace wifi_connection;

    class WiFiConnection
        : public task::Task<1024 * 8, task::core::System, task::priority::Background>
    {
    public:
        enum class Signal { Excellent, Good, Fair, Bad };

        void begin();
        void connect(const String& ssid, const String& pass);

        bool isConnecting() const { return m_connect.trying; }
        bool isConnectedToAP() const { return WiFi.isConnected(); }
        bool isInAccessPointMode() const { return !isConnectedToAP() && WiFi.getMode() == WIFI_MODE_AP; }
        bool isInternetAccessible() const { return m_internet.available(); }
    
        int8_t getSignalRSSI() const { return WiFi.RSSI(); }
        Signal getSignalQuality(int8_t rssi) const;

        WiFiConnectionUI& getUI() { return m_ui; }

    private:
        void task() override;
        void beginConnection();
        void updateConnection();

    private:
        WiFiConnectionUI m_ui;
        struct {
            String ssid;
            String pass;
            uint32_t tout = 0;
            uint32_t time = 0;
            bool trying = false;
        } m_connect; 
        InternetAccess m_internet;
    };

    extern WiFiConnection wifiConnection;
}
