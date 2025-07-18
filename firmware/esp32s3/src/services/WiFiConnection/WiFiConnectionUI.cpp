#include "WiFiConnectionUI.h"
#include "services/WiFiConnection.h"
#include "settings.h"

#define MAX_STA_COUNT 10
#define MIN_CONN_TOUT 10
#define MAX_CONN_TOUT 60

namespace service::details
{
    void WiFiConnectionUI::begin()
    {
        m_action = Action::GoToManual;
    }

    void WiFiConnectionUI::settingsBuild(sets::Builder& b)
    {
        b.beginGuest();
        sets::Group g(b, "📶 WiFi");
        switch (m_action)
        {
            case Action::DoScan:
                b.Label("Scanning...");
                break;

            case Action::DoConnect:
                b.Label("Connecting...");
                break;

            case Action::GoToManual:
                m_action = Action::None;
                m_ssid = Settings::data()[wifi::ssid];
                m_pass = Settings::data()[wifi::pass];
                m_stations.clear();
                // fall down
                
            default:
                if (m_stations.empty())
                {
                    b.Input("SSID", &m_ssid);
                }
                else
                {
                    String options;
                    fillOptionsWithStations(options);
                    if (b.Select("SSID", options))
                    {
                        chooseStationByIndex(b.build.value);
                    }
                }
                if (isPassClosedStation())
                {
                    b.Pass ("Password", &m_pass);
                }
                b.Slider(wifi::tout, "Connection timeout",
                    MIN_CONN_TOUT, MAX_CONN_TOUT, 5, " seconds");
                {
                    sets::Buttons buttons(b);
                    if (m_stations.empty())
                    {
                        if (b.Button("Scan"))
                        {    
                            m_action = Action::DoScan;
                            b.reload();
                        }
                    }
                    else
                    {
                        if (b.Button("Manual"))
                        {
                            m_action = Action::GoToManual;
                            b.reload();
                        }
                    }
                    if (b.Button("Connect")) 
                    {
                        m_action = Action::DoConnect;
                        b.reload();
                    }
                }
                break;
        }
        b.endGuest();
    }

    void WiFiConnectionUI::settingsUpdate(sets::Updater& u)
    {
        switch (m_action)
        {
            case Action::DoScan:
                scanForStations();
                chooseStationByIndex(0);
                m_action = Action::None;
                Settings::sets().reload();
                break;

            case Action::DoConnect:
                wifiConnection.connect(m_ssid, m_pass);
                m_action = Action::GoToManual;
                Settings::sets().reload();
                break;
        }
    }

    void WiFiConnectionUI::scanForStations()
    {
        WiFi.scanNetworks();
        const auto found = WiFi.scanComplete();

        m_stations.clear();
        for (int i = 0; i < found && m_stations.size() < MAX_STA_COUNT; ++i) 
        {
            const String ssid = WiFi.SSID(i);
            if (ssid.isEmpty()) continue;

            auto open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            auto signal = int(service::wifiConnection.getSignal(WiFi.RSSI(i)));
            m_stations.push_back({ ssid, open, signal });
        }

        WiFi.scanDelete();
    }

    bool WiFiConnectionUI::isPassClosedStation() const
    {
        for (auto& station : m_stations)
        {
            if (station.ssid == m_ssid) return !station.open;
        }
        return true;
    }

    void WiFiConnectionUI::fillOptionsWithStations(String& options)
    {
        for (auto& station : m_stations)
        {
            switch (station.signal)
            {
                case 0:
                case 1: options += led(Led::G) + " "; break;
                case 2: options += led(Led::Y) + " "; break;
                case 3: options += led(Led::R) + " "; break;
            }
            options += (station.open ? "🔓 " : "🔐 ") + station.ssid + ";";
        }
    }

    void WiFiConnectionUI::chooseStationByIndex(size_t index)
    {
        if (index < m_stations.size())
        {
            m_ssid = m_stations[index].ssid;
            m_pass = "";
        }
    }
}
