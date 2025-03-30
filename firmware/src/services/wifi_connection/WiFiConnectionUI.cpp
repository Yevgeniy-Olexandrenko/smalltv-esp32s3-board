#include "WiFiConnectionUI.h"
#include "settings.h"

namespace service::wifi_connection
{
    void WiFiConnectionUI::begin()
    {
        // set default and get current settings
        settings::data().init(db::wifi_ssid, "bla-wifi");
        settings::data().init(db::wifi_pass, "347279524");
        settings::data().init(db::wifi_tout, 20);
        m_ssid = settings::data()[db::wifi_ssid];
        m_pass = settings::data()[db::wifi_pass];

    }

    void WiFiConnectionUI::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "WiFi");
        if (isScanning())         
            b.Label("Scanning...");
        else if (isConnecting())
            b.Label("Connecting...");
        else
        {
            if (isManualInput())
                b.Input("SSID", &m_ssid);
            else
            {
                String options;
                std::vector<String> values;
                fetchWiFiStationsOptions(options, values);

                if (b.Select("SSID", options))
                {
                    m_ssid = values[b.build.value];
                    m_pass = "";
                }
            }
            b.Pass ("Password", &m_pass);
            b.Slider(db::wifi_tout, "Connection timeout", 10, 60, 5, " seconds");
            {
                sets::Buttons buttons(b);
                if (isManualInput())
                {
                    if (b.Button("Scan"))
                    {
                        // TODO
                        // m_state = State::ScanRequested;
                        // m_ssid = settings::data()[wifi::ssid];
                        // m_pass = settings::data()[wifi::pass];
                        // b.reload();

                        log_i("wifi start scanning");
                        WiFi.scanNetworks(true, false, false);
                        m_ssid = "";
                        m_pass = "";
                        b.reload();
                    }
                }
                else
                {
                    if (b.Button("Manual"))
                    {
                        WiFi.scanDelete();
                        m_ssid = settings::data()[db::wifi_ssid];
                        m_pass = settings::data()[db::wifi_pass];
                        b.reload();
                    }
                }
                
                if (b.Button("Connect")) 
                {
                    // TODO
                    // m_ssid.trim();
                    // m_state = State::ConnectRequested;
                    // settings::data()[wifi::ssid] = m_ssid;
                    // settings::data()[wifi::pass] = m_pass;
                    // settings::data().update();
                    // WiFi.scanDelete();
                    // b.reload();
                }
            }
        }





        
    }

    void WiFiConnectionUI::settingsUpdate(sets::Updater &u)
    {
        if (isScanning() || isConnecting())
        {
            settings::sets().reload();
        }
    }

    bool WiFiConnectionUI::isScanning() const
    {
        return (WiFi.scanComplete() == WIFI_SCAN_RUNNING);
    }

    bool WiFiConnectionUI::isConnecting() const
    {
        // TODO
        return false;
    }

    bool WiFiConnectionUI::isManualInput() const
    {
        int16_t result = WiFi.scanComplete();
        return (result == WIFI_SCAN_FAILED || result == 0);
    }

    void WiFiConnectionUI::fetchWiFiStationsOptions(String &options, std::vector<String>& values)
    {
        values.clear();
        const int16_t found = WiFi.scanComplete();

        for (int i = 0; i < found && values.size() < 10; i++) 
        {
            const String ssid = WiFi.SSID(i);
            if (ssid.isEmpty()) continue;

            options += WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "🔐" : "🔓";
            options += " " + ssid + ";";
            values.push_back(ssid);
        }
    }
}
