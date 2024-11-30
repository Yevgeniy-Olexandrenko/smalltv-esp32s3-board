#include "NetworkConnectionService.h"
#include <WiFiConnector.h>

DB_KEYS(wifi, ssid, pass);

void NetworkConnectionServiceClass::begin(GyverDBFile &db, sets::SettingsBase &sets)
{
    m_db = &db; m_sets = &sets;

    (*m_db).init(wifi::ssid, "");
    (*m_db).init(wifi::pass, "");
    m_ssid = (*m_db)[wifi::ssid];
    m_pass = (*m_db)[wifi::pass];

    WiFiConnector.onConnect([]() 
    {
        Serial.print("Connected! ");
        Serial.println(WiFi.localIP());
    });

    WiFiConnector.onError([]() 
    {
        Serial.print("Error! start AP ");
        Serial.println(WiFi.softAPIP());
    });

    //
    Serial.println("WiFi Connect start!");
    WiFiConnector.setName("SmallTV");
    WiFiConnector.setTimeout(15);

    //
    WiFiConnector.connect(m_ssid, m_pass);
    m_state = State::Connecting;
}

void NetworkConnectionServiceClass::settingsBuild(sets::Builder &b)
{
    b.beginGroup("WiFi");
    if (m_state == State::ConnectRequested)
        b.Label("Connecting...");
    else if (m_state == State::ScanRequested)
        b.Label("Scanning...");
    else
    {
        if (WiFi.scanComplete() > 0)
        {
            buildWiFiScanResult(b, 8);
            b.endGroup();
            b.beginGroup();
        }
        buildWiFiConnection(b);
    }
    b.endGroup();
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
            b.Label(ssid, WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "🔒" : "");
            if (b.Button("Select"))
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

    if (b.beginButtons())
    {
        if (b.Button("Scan"))
        {
            m_state = State::ScanRequested;
            m_ssid = (*m_db)[wifi::ssid];
            m_pass = (*m_db)[wifi::pass];
            b.reload();
        }
        if (b.Button("Connect")) 
        {
            m_state = State::ConnectRequested;
            (*m_db)[wifi::ssid] = m_ssid;
            (*m_db)[wifi::pass] = m_pass;
            (*m_db).update();
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
            Serial.println("WiFi Connect start!");
            WiFiConnector.connect(m_ssid, m_pass);
            m_state = State::Connecting;
            break;

        case State::Connecting:
            if (!WiFiConnector.connecting())
            {
                Serial.println("WiFi Connect finish!");
                m_state = (WiFiConnector.connected() 
                    ? State::Connected 
                    : State::NotConnected);
                (*m_sets).reload();
            }
            break;

        case State::ScanRequested:
            Serial.println("WiFi Scan start!");
            WiFi.scanNetworks(true, false, false);
            m_state = State::Scanning;
            break;

        case State::Scanning:
            if (WiFi.scanComplete() != WIFI_SCAN_RUNNING)
            {
                Serial.println("WiFi Scan finish!");
                (*m_sets).reload();
                m_state = State::Connecting;
            }
            break;
    }
}

NetworkConnectionServiceClass NetworkConnectionService;
