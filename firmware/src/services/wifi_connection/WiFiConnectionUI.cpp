#include "WiFiConnectionUI.h"
#include "WiFiConnection.h"
#include "settings.h"

namespace service::wifi_connection
{
    void WiFiConnectionUI::begin()
    {
        m_reqScan = false;
        m_reqConnect = false;
        m_reqGoToManual = true;
    }

    void WiFiConnectionUI::settingsBuild(sets::Builder &b)
    {
        b.beginGuest();
        sets::Group g(b, "📶 WiFi");

        if (m_reqGoToManual)
        {
            m_reqGoToManual = false;
            m_ssid = settings::data()[db::wifi_ssid];
            m_pass = settings::data()[db::wifi_pass];
            WiFi.scanDelete();
        }

        if (m_reqScan)     
            b.Label("Scanning...");
        else if (m_reqConnect)
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
                fetchSSIDScanResultOptions(options);
                if (b.Select("SSID", options))
                {
                    setSSIDFromScanResult(b.build.value);
                }
            }
            if (isAuthClosedNetwork())
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
                        m_reqScan = true;
                        b.reload();
                    }
                }
                else
                {
                    if (b.Button("Manual"))
                    {
                        m_reqGoToManual = true;
                        b.reload();
                    }
                }
                if (b.Button("Connect")) 
                {
                    m_reqConnect = true;
                    b.reload();
                }
            }
        }
        b.endGuest();
    }

    void WiFiConnectionUI::settingsUpdate(sets::Updater &u)
    {
        if (m_reqScan)
        {
            m_reqScan = false;
            WiFi.scanNetworks();
            setSSIDFromScanResult(0);
            settings::sets().reload();
        }

        if (m_reqConnect)
        {
            m_reqConnect = false;
            wifiConnection.connect(m_ssid, m_pass);
            m_reqGoToManual = true;
            settings::sets().reload();
        }
    }

    bool WiFiConnectionUI::isManualInput() const
    {
        int16_t result = WiFi.scanComplete();
        return (result == WIFI_SCAN_FAILED || result == 0);
    }

    bool WiFiConnectionUI::isAuthClosedNetwork() const
    {
        const int16_t found = WiFi.scanComplete();
        for (int i = 0; i < found; ++i) 
        {
            const String ssid = WiFi.SSID(i);
            if (!ssid.isEmpty() && ssid == m_ssid)
            {
                return (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            }
        }
        return true;
    }

    void WiFiConnectionUI::fetchSSIDScanResultOptions(String &options)
    {
        const int16_t found = WiFi.scanComplete();
        for (int i = 0, count = 0; i < found && count < 10; ++i) 
        {
            const String ssid = WiFi.SSID(i);
            if (ssid.isEmpty()) continue;

            auto open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            auto quality = int(service::wifiConnection.getSignal(WiFi.RSSI(i)));

            switch (quality)
            {
                case 0:
                case 1: options += led(Led::G) + " "; break;
                case 2: options += led(Led::Y) + " "; break;
                case 3: options += led(Led::R) + " "; break;
            }
            options += (open ? "🔓 " : "🔐 ") + ssid + ";";
            count++;
        }
    }

    void WiFiConnectionUI::setSSIDFromScanResult(size_t index)
    {
        std::vector<String> SSIDs;

        const int16_t found = WiFi.scanComplete();
        for (int i = 0; i < found && SSIDs.size() < 10; ++i) 
        {
            const String ssid = WiFi.SSID(i);
            if (ssid.isEmpty()) continue;
            SSIDs.push_back(ssid);
        }
        if (index < SSIDs.size())
        {
            m_ssid = SSIDs[index];
            m_pass = "";
            log_i("chosen ssid: %s", m_ssid);
        }
    }
}
