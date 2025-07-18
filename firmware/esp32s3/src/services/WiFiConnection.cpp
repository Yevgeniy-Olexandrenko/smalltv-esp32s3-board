#include <HTTPClient.h>
#include "WiFiConnection.h"
#include "firmware/defines.h"
#include "firmware/secrets.h"
#include "settings.h"

namespace service
{
    void WiFiConnection::begin()
    {
        settings::data().init(db::wifi_ssid, WIFI_SSID);
        settings::data().init(db::wifi_pass, WIFI_PASS);
        settings::data().init(db::wifi_tout, DEFAULT_CONNECT_TOUT_SEC);

        m_ui.begin();
        WiFi.setHostname((WIFI_HOST_NAME + getDeviceID()).c_str());
        WiFi.setAutoReconnect(true);
        Task::start("wifi_connection");

        log_i("connect to wifi on boot");
        String ssid = settings::data()[db::wifi_ssid];
        String pass = settings::data()[db::wifi_pass];
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
            m_connect.timer.start(settings::data()[db::wifi_tout]);

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
                settings::data()[db::wifi_ssid] = m_connect.ssid;
                settings::data()[db::wifi_pass] = m_connect.pass;
                settings::data().update();
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
