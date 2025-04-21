#include "WiFiConnectionUI.h"
#include "WiFiConnection.h"
#include "settings.h"

#define MAX_SSID_COUNT 10
#define MIN_CONN_TOUT  10
#define MAX_CONN_TOUT  60

namespace service::wifi_connection
{
    void WiFiConnectionUI::begin()
    {
        m_request = Request::GoToManual;
    }

    void WiFiConnectionUI::settingsBuild(sets::Builder& b)
    {
        b.beginGuest();
        sets::Group g(b, "📶 WiFi");
        switch (m_request)
        {
            case Request::GoToManual:
                m_request = Request::None;
                m_ssid = settings::data()[db::wifi_ssid];
                m_pass = settings::data()[db::wifi_pass];
                WiFi.scanDelete();
                break;

            case Request::Scan:
                b.Label("Scanning...");
                break;

            case Request::Connect:
                b.Label("Connecting...");
                break;

            default:
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
                b.Slider(db::wifi_tout, "Connection timeout",
                     MIN_CONN_TOUT, MAX_CONN_TOUT, 5, " seconds");
                {
                    sets::Buttons buttons(b);
                    if (isManualInput())
                    {
                        if (b.Button("Scan"))
                        {    
                            m_request = Request::Scan;
                            b.reload();
                        }
                    }
                    else
                    {
                        if (b.Button("Manual"))
                        {
                            m_request = Request::GoToManual;
                            b.reload();
                        }
                    }
                    if (b.Button("Connect")) 
                    {
                        m_request = Request::Connect;
                        b.reload();
                    }
                }
                break;
        }
        b.endGuest();
    }

    void WiFiConnectionUI::settingsUpdate(sets::Updater& u)
    {
        switch (m_request)
        {
            case Request::Scan:
                WiFi.scanNetworks();
                setSSIDFromScanResult(0);
                m_request = Request::None;
                settings::sets().reload();
                break;

            case Request::Connect:
                wifiConnection.connect(m_ssid, m_pass);
                m_request = Request::GoToManual;
                settings::sets().reload();
                break;
        }
    }

    bool WiFiConnectionUI::isManualInput() const
    {
        const auto result = WiFi.scanComplete();
        return (result == WIFI_SCAN_FAILED || result == 0);
    }

    bool WiFiConnectionUI::isAuthClosedNetwork() const
    {
        const auto found = WiFi.scanComplete();
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

    void WiFiConnectionUI::fetchSSIDScanResultOptions(String& options)
    {
        const auto found = WiFi.scanComplete();
        for (int i = 0, count = 0; i < found && count < MAX_SSID_COUNT; ++i) 
        {
            const String ssid = WiFi.SSID(i);
            if (ssid.isEmpty()) continue;

            auto open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            auto signal = int(service::wifiConnection.getSignal(WiFi.RSSI(i)));

            switch (signal)
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
        const auto found = WiFi.scanComplete();
        for (int i = 0; i < found && SSIDs.size() < MAX_SSID_COUNT; ++i) 
        {
            const String ssid = WiFi.SSID(i);
            if (!ssid.isEmpty()) SSIDs.push_back(ssid);
        }
        if (index < SSIDs.size())
        {
            m_ssid = SSIDs[index];
            m_pass = "";
        }
    }
}
