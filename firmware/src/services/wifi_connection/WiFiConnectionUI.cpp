#include "WiFiConnectionUI.h"
#include "WiFiConnection.h"
#include "settings.h"

namespace service::wifi_connection
{
    void WiFiConnectionUI::begin()
    {
        m_gotoManualReq = true;
        m_scanRequested = false;
        m_connRequested = false;
    }

    void WiFiConnectionUI::settingsBuild(sets::Builder &b)
    {
        b.beginGuest();
        sets::Group g(b, "📶 WiFi");

        if (m_gotoManualReq)
        {
            m_gotoManualReq = false;
            m_ssid = settings::data()[db::wifi_ssid];
            m_pass = settings::data()[db::wifi_pass];
            WiFi.scanDelete();
        }

        if (m_scanRequested)     
            b.Label("Scanning...");
        else if (m_connRequested)
            b.Label("Connecting...");
        else
        {
            if (isManualInput())
            {
                b.Input("SSID", &m_ssid);
            }
            else
            {
                String options;
                std::vector<String> values;
                fetchAvailableAPOptions(options, values);

                if (b.Select("SSID", options))
                {
                    m_ssid = values[b.build.value];
                    m_pass = "";
                    log_i("chosen ssid: %s", m_ssid);
                }
            }
            if (isClosedNetwork())
            {
                b.Pass ("Password", &m_pass);
            }
            b.Slider(db::wifi_tout, "Connection timeout", 10, 60, 5, " seconds");
            {
                sets::Buttons buttons(b);
                if (isManualInput())
                {
                    if (b.Button("Scan"))
                    {    
                        m_scanRequested = true;
                        b.reload();
                    }
                }
                else
                {
                    if (b.Button("Manual"))
                    {
                        m_gotoManualReq = true;
                        b.reload();
                    }
                }
                if (b.Button("Connect")) 
                {
                    m_connRequested = true;
                    b.reload();
                }
            }
        }
        b.endGuest();
    }

    void WiFiConnectionUI::settingsUpdate(sets::Updater &u)
    {
        if (m_scanRequested)
        {
            m_scanRequested = false;
            WiFi.scanNetworks();

            if (!isManualInput())
            {
                String options;
                std::vector<String> values;
                fetchAvailableAPOptions(options, values);

                m_ssid = values[0];
                m_pass = "";
                log_i("chosen ssid: %s", m_ssid);
            }
            settings::sets().reload();
        }

        if (m_connRequested)
        {
            m_connRequested = false;
            wifiConnection.connect(m_ssid, m_pass);

            m_gotoManualReq = true;
            settings::sets().reload();
        }
    }

    bool WiFiConnectionUI::isManualInput() const
    {
        int16_t result = WiFi.scanComplete();
        return (result == WIFI_SCAN_FAILED || result == 0);
    }

    bool WiFiConnectionUI::isClosedNetwork() const
    {
        const int16_t found = WiFi.scanComplete();
        for (int i = 0; i < found; i++) 
        {
            const String ssid = WiFi.SSID(i);
            if (!ssid.isEmpty() && ssid == m_ssid)
            {
                return (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            }
        }
        return true;
    }

    void WiFiConnectionUI::fetchAvailableAPOptions(String &options, std::vector<String> &values)
    {
        const int16_t found = WiFi.scanComplete();
        for (int i = 0; i < found && values.size() < 10; i++) 
        {
            const String ssid = WiFi.SSID(i);
            if (ssid.isEmpty()) continue;

            auto open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            auto quality = int(service::wifiConnection.getSignalQuality(WiFi.RSSI(i)));

            switch (quality)
            {
                case 0:
                case 1: options += led(Led::G) + " "; break;
                case 2: options += led(Led::Y) + " "; break;
                case 3: options += led(Led::R) + " "; break;
            }
            options += (open ? "🔓 " : "🔐 ") + ssid + ";";
            values.push_back(ssid);
        }
    }
}
