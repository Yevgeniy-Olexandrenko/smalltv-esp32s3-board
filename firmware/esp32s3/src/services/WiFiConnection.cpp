#include <HTTPClient.h>
#include "WiFiConnection.h"
#include "settings/Settings.h"
#include "firmware/defines.h"
#include "firmware/secrets.h"

namespace service
{
    void WiFiConnection::begin()
    {
        Settings::data().init(wifi::ssid, WIFI_SSID);
        Settings::data().init(wifi::pass, WIFI_PASS);
        Settings::data().init(wifi::tout, DEFAULT_CONNECT_TOUT_SEC);

        m_ui.begin();
        WiFi.setHostname((WIFI_HOST_NAME + getDeviceID()).c_str());
        WiFi.setAutoReconnect(true);
        Task::start("wifi_connection");

        log_i("connect to wifi on boot");
        String ssid = Settings::data()[wifi::ssid];
        String pass = Settings::data()[wifi::pass];
        connect(ssid, pass);
    }

    void WiFiConnection::connect(const String& ssid, const String& pass)
    {
        m_connect.ssid = ssid;
        m_connect.pass = pass;
        m_connect.ssid.trim();
        m_connect.pass.trim();
        beginConnection();
    }

    WiFiConnection::Signal WiFiConnection::getSignal(int8_t rssi) const
    {
        if (rssi >= -50) return Signal::Excellent;
        if (rssi >= -70) return Signal::Good;
        if (rssi >= -80) return Signal::Fair;
        return Signal::Bad;
    }

    String WiFiConnection::getDeviceID() const
    {
        String mac = WiFi.macAddress();
        return mac.substring(12, 14) + mac.substring(15);
    }

    String WiFiConnection::getUserAgent() const
    { 
        return 
            WIFI_HOST_NAME + getDeviceID() + "/"
            PROJECT_VERSION " (contact: " PROJECT_AUTHOR ")";
    }

    void WiFiConnection::task()
    {
        while (true)
        {
            updateConnection();
            updateInternet();
            sleep(1000);
        }
    }

    void WiFiConnection::beginConnection()
    {
        if (m_connect.ssid.isEmpty())
        {
            // new network is not set, so do not 
            // try to connect, just turn on the AP
            WiFi.mode(WIFI_AP);
            WiFi.softAP(WiFi.getHostname(), AP_PASS);
            m_connect.timer.stop();

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
            WiFi.softAP(WiFi.getHostname(), AP_PASS);
            WiFi.begin(m_connect.ssid, m_connect.pass);
            m_connect.timer.start(Settings::data()[wifi::tout]);

            log_i("start AP+STA: %s (%s)", 
                WiFi.softAPSSID().c_str(), 
                WiFi.softAPIP().toString().c_str());
        }
    }

    void WiFiConnection::updateConnection()
    {
        if (m_connect.timer.active()) 
        {
            if (WiFi.isConnected()) 
            {
                // save the current connected network
                // for future possible rollback
                Settings::data()[wifi::ssid] = m_connect.ssid;
                Settings::data()[wifi::pass] = m_connect.pass;
                Settings::data().update();
                m_connect.timer.stop();

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
            else if (m_connect.timer.expired()) 
            {
                // trying to rollback to a previously
                // connected network
                String ssid = Settings::data()[wifi::ssid];
                String pass = Settings::data()[wifi::pass];

                if (ssid.isEmpty())
                    log_i("rollback impossible");
                else
                    log_i("try rollback to: %s", ssid.c_str());

                // disable rollback repeat and connect
                Settings::data()[wifi::ssid] = "";
                Settings::data()[wifi::pass] = "";
                Settings::data().update();
                connect(ssid, pass);
            }
        }
    }

    void WiFiConnection::updateInternet()
    {
        if (isConnectedToAP())
        {
            if (m_internet.timer.expired())
            {
                // restart timer to wait for the next check
                m_internet.timer.start();

                // try to check if internet is available
                HTTPClient http;
                http.setConnectTimeout(1000 * INET_CHECK_TIMEOUT_SEC);
                http.begin("http://clients3.google.com/generate_204");
                m_internet.available = (http.GET() == 204);
                http.end();
            }
        }
        else
        {
            // invalidate for instant check
            m_internet.timer.stop();
            m_internet.available = false;
        }
    }

    WiFiConnection wifiConnection;
}
