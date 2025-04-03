#include "WiFiConnection.h"
#include "settings.h"
#include "defines.h"

namespace service
{
    void WiFiConnection::begin()
    {
        settings::data().init(db::wifi_ssid, "");
        settings::data().init(db::wifi_pass, "");
        settings::data().init(db::wifi_tout, 20);
 
        m_ui.begin();
        WiFi.setHostname(NETWORK_HOST_NAME);
        WiFi.setAutoReconnect(true);
 
        log_i("connect to wifi on boot");
        connect(settings::data()[db::wifi_ssid], settings::data()[db::wifi_pass]);
        Task::start("wifi_connection");
    }

    void WiFiConnection::connect(const String &ssid, const String &pass)
    {
        uint8_t tout = settings::data()[db::wifi_tout];
        m_connect.tout = tout * 1000ul;
        m_connect.ssid = ssid;
        m_connect.pass = pass;
        m_connect.ssid.trim();
        beginConnection();
    }

    WiFiConnection::Signal WiFiConnection::getSignalQuality(int8_t rssi) const
    {
        if (rssi >= -50) return Signal::Excellent;
        if (rssi >= -70) return Signal::Good;
        if (rssi >= -80) return Signal::Fair;
        return Signal::Bad;
    }

    void WiFiConnection::task()
    {
        while (true)
        {
            updateConnection();
            m_internet.update();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    void WiFiConnection::beginConnection()
    {
        if (m_connect.ssid.isEmpty())
        {
            // new network is not set, so do not 
            // try to connect, just turn on the AP
            WiFi.mode(WIFI_AP);
            WiFi.softAP(NETWORK_ACCESS_POINT, "");
            m_connect.trying = false;

            log_i("start AP: %s (%s)", 
                WiFi.softAPSSID().c_str(), 
                WiFi.softAPIP().toString().c_str());
        }
        else
        {
            log_i("try connect to: %s", m_connect.ssid.c_str());

            // start connection attempt, turn on the 
            // AP and try to  connect to a new network
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(NETWORK_ACCESS_POINT, "");
            WiFi.begin(m_connect.ssid, m_connect.pass);
            m_connect.time = millis();
            m_connect.trying = true;

            log_i("start AP+STA: %s (%s)", 
                WiFi.softAPSSID().c_str(), 
                WiFi.softAPIP().toString().c_str());
        }
    }

    void WiFiConnection::updateConnection()
    {
        if (m_connect.trying) 
        {
            if (WiFi.isConnected()) 
            {
                // save the current connected network
                // for future possible rollback
                settings::data()[db::wifi_ssid] = m_connect.ssid;
                settings::data()[db::wifi_pass] = m_connect.pass;
                settings::data().update();
                m_connect.trying = false;

                log_i("connected to: %s (%s)",
                    WiFi.SSID().c_str(),
                    WiFi.localIP().toString().c_str());

                // turn off the AP mode
                if (WiFi.getMode() == WIFI_AP_STA) 
                {
                    WiFi.softAPdisconnect(true);
                    WiFi.mode(WIFI_STA);

                    log_i("stop AP");
                }
            } 
            else if (millis() - m_connect.time >= m_connect.tout) 
            {
                // trying to rollback to a previously
                // connected network
                String ssid = settings::data()[db::wifi_ssid];
                String pass = settings::data()[db::wifi_pass];

                if (ssid.isEmpty())
                    log_i("rollback impossible");
                else
                    log_i("try rollback to: %s", ssid.c_str());

                // disable rollback repeat and connect
                settings::data()[db::wifi_ssid] = "";
                settings::data()[db::wifi_pass] = "";
                settings::data().update();
                connect(ssid, pass);
            }
        }
    }

    WiFiConnection wifiConnection;
}
