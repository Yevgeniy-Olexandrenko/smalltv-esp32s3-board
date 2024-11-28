#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <WiFiConnector.h>

GyverDBFile settingsDB(&LittleFS, "/settings.db");
SettingsGyver settings("📺 SmallTV Settings", &settingsDB);

String _ssid;
String _pass;

enum class State { ConnectRequested, Connecting, Connected, NotConnected, ScanRequested, Scanning };
State _state = State::NotConnected;

DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    apply);

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
                _ssid = ssid;
                _pass = "";
                b.reload();
            }
            b.endRow();
            count++;
        }
    }
}

void buildWiFiConnection(sets::Builder& b)
{
    b.Input("SSID", &_ssid);
    b.Pass ("Password", &_pass);

    if (b.beginButtons())
    {
        if (b.Button("Scan"))
        {
            _state = State::ScanRequested;
            _ssid = settingsDB[kk::wifi_ssid];
            _pass = settingsDB[kk::wifi_pass];
            b.reload();
        }
        if (b.Button("Connect")) 
        {
            _state = State::ConnectRequested;
            settingsDB[kk::wifi_ssid] = _ssid;
            settingsDB[kk::wifi_pass] = _pass;
            settingsDB.update();
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
    settingsDB.begin();
    settingsDB.init(kk::wifi_ssid, "");
    settingsDB.init(kk::wifi_pass, "");
    _ssid = settingsDB[kk::wifi_ssid];
    _pass = settingsDB[kk::wifi_pass];

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
    WiFiConnector.connect(_ssid, _pass);
    _state = State::Connecting;

    // запускаем сервер после connect, иначе DNS не подхватится
    settings.begin();
    settings.onBuild(build);
    settings.onUpdate(update);
}

void loop() 
{
    WiFiConnector.tick();
    settings.tick();

    switch(_state)
    {
        case State::ConnectRequested:
            Serial.println("WiFi Connect start!");
            WiFiConnector.connect(_ssid, _pass);
            _state = State::Connecting;
            break;

        case State::Connecting:
            if (!WiFiConnector.connecting())
            {
                Serial.println("WiFi Connect finish!");
                _state = (WiFiConnector.connected() 
                    ? State::Connected 
                    : State::NotConnected);
                settings.reload();
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
                settings.reload();
                _state = State::Connecting;
            }
            break;
    }
}
