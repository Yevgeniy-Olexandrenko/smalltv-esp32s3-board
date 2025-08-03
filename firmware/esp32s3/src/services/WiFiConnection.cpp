#include <HTTPClient.h>
#include "WiFiConnection.h"
#include "settings/Settings.h"
#include "firmware/secrets.h"

namespace service
{
    void WiFiConnection::begin()
    {
        Settings::data().init(wifi::ssid, WIFI_SSID);
        Settings::data().init(wifi::pass, WIFI_PASS);
        Settings::data().init(wifi::tout, DEFAULT_CONNECT_TOUT_SEC);

        m_ui.begin();
        WiFi.setMinSecurity(WIFI_AUTH_WEP);
        WiFi.setHostname(Settings::getHostName().c_str());
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
        if (!m_connect.ssid.isEmpty())
        {
            // start connection attempt
            m_connect.trying = true;
            m_connect.timer.start(Settings::data()[wifi::tout]);
            log_i("try connect to: %s", m_connect.ssid.c_str());

            // trying to connect to STA
            WiFi.softAPdisconnect(true);
            WiFi.disconnect(true);
            WiFi.mode(WIFI_STA);
            WiFi.begin(m_connect.ssid, m_connect.pass);
        }
        else
        {
            // network SSID is not set, so do not 
            // try to connect, just turn on the AP
            m_connect.trying = false;
            m_connect.rollback = false;

            // trying to start the AP
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP);
            WiFi.softAP(WiFi.getHostname(), AP_PASS);
            log_i("start AP: %s (%s)", 
                WiFi.softAPSSID().c_str(), 
                WiFi.softAPIP().toString().c_str());
        }
    }

    void WiFiConnection::updateConnection()
    {
        if (m_connect.trying)
        {
            if (m_connect.timer.active())
            {
                // trying to connect
                if (WiFi.isConnected())
                {
                    // connection is established
                    m_connect.trying = false;
                    m_connect.rollback = false;
                    log_i("connected to: %s (%s)",
                        WiFi.SSID().c_str(),
                        WiFi.localIP().toString().c_str());

                    // save the current connected network
                    // for future connect or possible rollback
                    Settings::data()[wifi::ssid] = m_connect.ssid;
                    Settings::data()[wifi::pass] = m_connect.pass;
                    Settings::data().update();
                }
            }
            else if (!m_connect.rollback)
            {
                // connection timeout expired, trying to 
                // rollback to a previously connected STA
                String ssid = Settings::data()[wifi::ssid];
                String pass = Settings::data()[wifi::pass];
                log_i("try rollback to: %s", ssid.c_str());
                m_connect.rollback = true;
                connect(ssid, pass);
            }
            else
            {
                log_i("rollback failed");
                m_connect.rollback = false;
                connect("", "");
            }
        }
    }

    void WiFiConnection::updateInternet()
    {
        if (WiFi.isConnected())
        {
            if (m_internet.timer.expired())
            {
                // restart timer to wait for the next check
                m_internet.timer.start();

                // try to check if internet is available
                HTTPClient http;
                http.setConnectTimeout(1000 * INET_CHECK_TIMEOUT_SEC);
                http.begin("http://clients3.google.com/generate_204");
                http.addHeader("User-Agent", Settings::getUserAgent());
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
