#include <WiFiConnector.h>
#include "NetworkConnection.h"
#include "shared/settings/Settings.h"
#include "drivers/onboard/_LedAndButton.h"
#include "defines.h"

namespace service
{
    DB_KEYS(wifi, ssid, pass, tout);

    void NetworkConnection::begin()
    {
        Serial.println("NetworkConnection: begin");

        // set default and get current settings
        settings::data().init(wifi::ssid, "");
        settings::data().init(wifi::pass, "");
        settings::data().init(wifi::tout, 20);
        m_ssid = settings::data()[wifi::ssid];
        m_pass = settings::data()[wifi::pass];

        // configure AP name and connection timeout
        WiFi.setHostname(NETWORK_HOST_NAME);
        WiFiConnector.setName(NETWORK_ACCESS_POINT);
        WiFiConnector.setTimeout(settings::data()[wifi::tout]);

        // try to connect on firmware startup
        WiFiConnector.onConnect([]() 
        {
            Serial.print("NetworkConnection: connected: ");
            Serial.println(WiFi.localIP());
            driver::ledAndButton.setLedMode(driver::LedAndButton::LedMode::On);
        });

        WiFiConnector.onError([]() 
        {
            Serial.print("NetworkConnection: failed, start AP: ");
            Serial.println(WiFi.softAPIP());
            driver::ledAndButton.setLedMode(driver::LedAndButton::LedMode::Blink);
        });

        Serial.println("NetworkConnection: connect on boot");
        driver::ledAndButton.setLedMode(driver::LedAndButton::LedMode::Off);
        WiFiConnector.connect(m_ssid, m_pass);
        m_state = State::Connecting;
    }

    void NetworkConnection::settingsBuild(sets::Builder &b)
    { 
        if (m_state == State::ConnectRequested)
        {
            sets::Group g(b, "WiFi");
            b.Label("Connecting...");
        }
        else if (m_state == State::ScanRequested)
        {
            sets::Group g(b, "WiFi");
            b.Label("Scanning...");
        }
        else
        {
            if (WiFi.scanComplete() > 0)
            {
                sets::Group g(b, "WiFi stations");
                buildWiFiScanResult(b, 8);
            }
            sets::Group g(b, "WiFi connection");
            buildWiFiConnection(b);
        }
    }

    void NetworkConnection::settingsUpdate(sets::Updater &u)
    {
        // do nothing for now
    }

    int NetworkConnection::getSignalRSSI() const
    {
        return WiFi.RSSI();
    }

    NetworkConnection::Signal NetworkConnection::getSignalStrength() const
    {
        const int rssi = getSignalRSSI();
        if (rssi >= -50) return Signal::Excellent;
        if (rssi >= -70) return Signal::Good;
        if (rssi >= -80) return Signal::Fair;
        return Signal::Bad;
    }

    bool NetworkConnection::isInAccessPointMode() const
    {
        return !WiFiConnector.connected();
    }

    bool NetworkConnection::isInternetAccessible() const
    {
        return m_internetAccess.available();
    }

    void NetworkConnection::buildWiFiScanResult(sets::Builder &b, int max)
    {
        const int16_t found = WiFi.scanComplete();
        for (int i = 0, count = 0; i < found && count < max; i++) 
        {
            const String ssid = WiFi.SSID(i);
            if (ssid.length())
            {
                b.beginRow();
                b.Label(ssid, WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "🔐" : "🔓");
                if (b.Button("Set"))
                {
                    WiFi.scanDelete();
                    m_ssid = ssid;
                    m_pass = "";
                    b.reload();
                }
                b.endRow();
                count++;
            }
        }
    }

    void NetworkConnection::buildWiFiConnection(sets::Builder &b)
    {
        b.Input("SSID", &m_ssid);
        b.Pass ("Password", &m_pass);
        b.Slider(
            wifi::tout, "Connection timeout",
            NETWORK_CON_TOUT_MIN, NETWORK_CON_TOUT_MAX, 5,
            " seconds"
        );
        if (b.beginButtons())
        {
            if (b.Button("Scan"))
            {
                m_state = State::ScanRequested;
                m_ssid = settings::data()[wifi::ssid];
                m_pass = settings::data()[wifi::pass];
                b.reload();
            }
            if (b.Button("Connect")) 
            {
                m_ssid.trim();
                m_state = State::ConnectRequested;
                settings::data()[wifi::ssid] = m_ssid;
                settings::data()[wifi::pass] = m_pass;
                settings::data().update();
                WiFi.scanDelete();
                b.reload();
            }
            b.endButtons();
        }
    }

    void NetworkConnection::update()
    {
        WiFiConnector.tick();

        if (WiFiConnector.connected())
        {
            m_internetAccess.update();
        }
        
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

    NetworkConnection networkConnection;
}
