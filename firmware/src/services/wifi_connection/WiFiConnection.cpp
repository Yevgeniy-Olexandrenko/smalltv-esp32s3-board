#include <WiFiConnector.h>
#include "WiFiConnection.h"
#include "settings.h"
#include "defines.h"

namespace service
{
    void WiFiConnection::begin()
    {
        log_i("start wifi");
        //Task::start("wifi_connection");

        m_ui.begin();

        // configure AP name and connection timeout
        WiFi.setHostname(NETWORK_HOST_NAME);
        WiFiConnector.setName(NETWORK_ACCESS_POINT);
        WiFiConnector.setTimeout(settings::data()[db::wifi_tout]);

        // try to connect on firmware startup
        WiFiConnector.onConnect([]() 
        {
            log_i("wifi connected: %s", WiFi.localIP().toString().c_str());
            // TODO
        });

        WiFiConnector.onError([]() 
        {
            log_i("wifi connection failed, start ap: %s", WiFi.softAPIP().toString().c_str());
            // TODO
        });

        log_i("wifi connection on boot");
        WiFiConnector.connect(m_ui.getSSID(), m_ui.getPass());
    }

    void WiFiConnection::update()
    {
        // TODO

        WiFiConnector.tick();

        switch(m_state)
        {
            case State::ConnectRequested:
                Serial.println("NetworkConnection: connect");
                driver::ledAndButton.setLedMode(driver::LedAndButton::LedMode::Off);
                WiFiConnector.connect(m_ssid, m_pass);
                m_state = State::Connecting;
                break;

            case State::Connecting:
                if (!WiFiConnector.connecting())
                {
                    m_state = (WiFiConnector.connected() 
                        ? State::Connected 
                        : State::NotConnected);
                    settings::sets().reload();
                }
                break;

            case State::ScanRequested:
                Serial.println("NetworkConnection: scan start");
                WiFi.scanNetworks(true, false, false);
                m_state = State::Scanning;
                break;

            case State::Scanning:
                if (WiFi.scanComplete() != WIFI_SCAN_RUNNING)
                {
                    Serial.println("NetworkConnection: scan finish");
                    settings::sets().reload();
                    m_state = State::Connecting;
                }
                break;
        }
    }

    int WiFiConnection::getSignalRSSI() const
    {
        return WiFi.RSSI();
    }

    WiFiConnection::Signal WiFiConnection::getSignalStrength() const
    {
        const int rssi = getSignalRSSI();
        if (rssi >= -50) return Signal::Excellent;
        if (rssi >= -70) return Signal::Good;
        if (rssi >= -80) return Signal::Fair;
        return Signal::Bad;
    }

    bool WiFiConnection::isInAccessPointMode() const
    {
        // TODO
        return false;
    }

    bool WiFiConnection::isInternetAccessible() const
    {
        // TODO
        return false;
    }

    void WiFiConnection::task()
    {
        // TODO
    }

    void WiFiConnection::beginConnection(const String &ssid, const String &pass, uint8_t tout)
    {
        if (ssid.isEmpty())
        {
            m_conTrying = false;
            startAccessPoint();
        }
        else
        {
            m_conTrying  = true;
            m_conStartTS = millis();
            m_conTimeout = tout * 1000ul;
            startStationAndAccessPoint(ssid.c_str(), pass.c_str());
        }
    }

    void WiFiConnection::updateConnection()
    {
        if (m_conTrying) 
        {
            if (isConnected()) 
            {
                m_conTrying = false;
                stopAccessPoint();
            } 
            else if (millis() - m_conStartTS >= m_conTimeout) 
            {
                m_conTrying = false;
                startAccessPoint();
            }
        }
    }

    bool WiFiConnection::isConnected() const
    {
        return (WiFi.status() == WL_CONNECTED);
    }

    void WiFiConnection::startStationAndAccessPoint(const char *ssid, const char *pass)
    {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(NETWORK_ACCESS_POINT, "");
        WiFi.begin(ssid, pass);
    }

    void WiFiConnection::startAccessPoint()
    {
        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        WiFi.softAP(NETWORK_ACCESS_POINT, "");
    }

    void WiFiConnection::stopAccessPoint()
    {
        WiFi.softAPdisconnect(true);
    }

    WiFiConnection wifiConnection;
}
