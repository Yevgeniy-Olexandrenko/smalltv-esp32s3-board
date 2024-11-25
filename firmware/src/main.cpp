#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsESP.h>
#include <WiFiConnector.h>

GyverDBFile settingsDB(&LittleFS, "/settings.db");
SettingsESP settings("SmallTV Settings", &settingsDB);

DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    apply);

void build(sets::Builder& b) 
{
    {
        sets::Group g(b, "WiFi");
        b.Input(kk::wifi_ssid, "SSID");
        b.Pass(kk::wifi_pass, "Password");

        if (b.Button(kk::apply, "Connect")) 
        {
            settingsDB.update();
            WiFiConnector.connect(
              settingsDB[kk::wifi_ssid],
              settingsDB[kk::wifi_pass]);
        }
    }
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

    WiFiConnector.connect(
      settingsDB[kk::wifi_ssid],
      settingsDB[kk::wifi_pass]);

    // запускаем сервер после connect, иначе DNS не подхватится
    settings.begin();
    settings.onBuild(build);
}

void loop() 
{
    WiFiConnector.tick();
    settings.tick();
}
