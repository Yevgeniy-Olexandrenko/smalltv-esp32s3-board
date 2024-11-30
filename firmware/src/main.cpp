#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <WiFiConnector.h>

#include "services/NetworkConnectionService.h"
#include "services/DateAndTimeService.h"
#include "services/GeoLocationService.h"
#include "services/WeatherService.h"

GyverDBFile m_db(&LittleFS, "/settings.db");
SettingsGyver m_sets("📺 SmallTV Settings", &m_db);

String m_ssid;
String m_pass;

enum class State { ConnectRequested, Connecting, Connected, NotConnected, ScanRequested, Scanning };
State _state = State::NotConnected;

DB_KEYS(wifi, ssid, pass);

void buildWiFiScanResult(sets::Builder& b, int max)
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

void buildWiFiConnection(sets::Builder& b)
{
    b.Input("SSID", &m_ssid);
    b.Pass ("Password", &m_pass);

    if (b.beginButtons())
    {
        if (b.Button("Scan"))
        {
            _state = State::ScanRequested;
            m_ssid = m_db[wifi::ssid];
            m_pass = m_db[wifi::pass];
            b.reload();
        }
        if (b.Button("Connect")) 
        {
            _state = State::ConnectRequested;
            m_db[wifi::ssid] = m_ssid;
            m_db[wifi::pass] = m_pass;
            m_db.update();
            WiFi.scanDelete();
            b.reload();
        }
        b.endButtons();
    }
}

void build(sets::Builder& b) 
{
    b.beginGroup("WiFi");
    if (_state == State::ConnectRequested)
        b.Label("Connecting...");
    else if (_state == State::ScanRequested)
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
    
void update(sets::Updater& u)
{
    //
}

void setup() 
{
    Serial.begin(115200);
    Serial.println();

    // базу данных запускаем до подключения к точке
    LittleFS.begin(true);
    m_db.begin();
    m_db.init(wifi::ssid, "");
    m_db.init(wifi::pass, "");
    m_ssid = m_db[wifi::ssid];
    m_pass = m_db[wifi::pass];

    // подключение и реакция на подключение или ошибку
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

    Serial.println("WiFi Connect start!");
    WiFiConnector.setName("SmallTV");
    WiFiConnector.setTimeout(10);
    WiFiConnector.connect(m_ssid, m_pass);
    _state = State::Connecting;

    // запускаем сервер после connect, иначе DNS не подхватится
    m_sets.begin();
    m_sets.onBuild(build);
    m_sets.onUpdate(update);
}

void loop() 
{
    WiFiConnector.tick();
    m_sets.tick();

    switch(_state)
    {
        case State::ConnectRequested:
            Serial.println("WiFi Connect start!");
            WiFiConnector.connect(m_ssid, m_pass);
            _state = State::Connecting;
            break;

        case State::Connecting:
            if (!WiFiConnector.connecting())
            {
                Serial.println("WiFi Connect finish!");
                _state = (WiFiConnector.connected() 
                    ? State::Connected 
                    : State::NotConnected);
                m_sets.reload();
            }
            break;

        case State::ScanRequested:
            Serial.println("WiFi Scan start!");
            WiFi.scanNetworks(true, false, false);
            _state = State::Scanning;
            break;

        case State::Scanning:
            if (WiFi.scanComplete() != WIFI_SCAN_RUNNING)
            {
                Serial.println("WiFi Scan finish!");
                m_sets.reload();
                _state = State::Connecting;
            }
            break;
    }
}
