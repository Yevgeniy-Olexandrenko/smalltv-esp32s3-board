#include "NetworkConnectionService.h"
#include "services/Settings/SettingsService.h"
#include <WiFiConnector.h>
#include "firmware.h"

DB_KEYS(wifi, ssid, pass, tout);

void NetworkConnectionServiceClass::begin()
{
    Serial.println("NetworkConnectionService: begin");

    // set default and get current settings
    SettingsService.data().init(wifi::ssid, "");
    SettingsService.data().init(wifi::pass, "");
    SettingsService.data().init(wifi::tout, 20);
    m_ssid = SettingsService.data()[wifi::ssid];
    m_pass = SettingsService.data()[wifi::pass];

    // configure AP name and connection timeout
    WiFiConnector.setName(NETWORK_ACCESS_POINT);
    WiFiConnector.setTimeout(SettingsService.data()[wifi::tout]);

    // try to connect on firmware startup
    Serial.println("NetworkConnectionService: connect");
    WiFiConnector.onConnect([]() 
    {
        Serial.print("NetworkConnectionService: connected: ");
        Serial.println(WiFi.localIP());
    });

    WiFiConnector.onError([]() 
    {
        Serial.print("NetworkConnectionService: failed, start AP: ");
        Serial.println(WiFi.softAPIP());
    });
    WiFiConnector.connect(m_ssid, m_pass);
    m_state = State::Connecting;
}

void NetworkConnectionServiceClass::settingsBuild(sets::Builder &b)
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

void NetworkConnectionServiceClass::settingsUpdate(sets::Updater &u)
{
    // do nothing for now
}

void NetworkConnectionServiceClass::buildWiFiScanResult(sets::Builder &b, int max)
{
    const int16_t found = WiFi.scanComplete();
    for (int i = 0, count = 0; i < found && count < max; i++) 
    {
        const String ssid = WiFi.SSID(i);
        if (ssid.length())
        {
            b.beginRow();
            b.Label(ssid, WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "🔐" : "🔓");
            if (b.Button("Select↩"))
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

void NetworkConnectionServiceClass::buildWiFiConnection(sets::Builder &b)
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
            m_ssid = SettingsService.data()[wifi::ssid];
            m_pass = SettingsService.data()[wifi::pass];
            b.reload();
        }
        if (b.Button("Connect")) 
        {
            m_ssid.trim();
            m_state = State::ConnectRequested;
            SettingsService.data()[wifi::ssid] = m_ssid;
            SettingsService.data()[wifi::pass] = m_pass;
            SettingsService.data().update();
            WiFi.scanDelete();
            b.reload();
        }
        b.endButtons();
    }
}

void NetworkConnectionServiceClass::update()
{
    WiFiConnector.tick();
    switch(m_state)
    {
        case State::ConnectRequested:
            Serial.println("NetworkConnectionService: connect");
            WiFiConnector.connect(m_ssid, m_pass);
            m_state = State::Connecting;
            break;

        case State::Connecting:
            if (!WiFiConnector.connecting())
            {
                m_state = (WiFiConnector.connected() 
                    ? State::Connected 
                    : State::NotConnected);
                SettingsService.sets().reload();
            }
            break;

        case State::ScanRequested:
            Serial.println("NetworkConnectionService: scan start");
            WiFi.scanNetworks(true, false, false);
            m_state = State::Scanning;
            break;

        case State::Scanning:
            if (WiFi.scanComplete() != WIFI_SCAN_RUNNING)
            {
                Serial.println("NetworkConnectionService: scan finish");
                SettingsService.sets().reload();
                m_state = State::Connecting;
            }
            break;
    }
}

NetworkConnectionServiceClass NetworkConnectionService;
