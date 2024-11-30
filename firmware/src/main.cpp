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

void build(sets::Builder& b) 
{
    NetworkConnectionService.settingsBuild(b);

    // TODO
}
    
void update(sets::Updater& u)
{
    NetworkConnectionService.settingsUpdate(u);

    // TODO
}

void setup() 
{
    Serial.begin(115200);
    Serial.println();

    // базу данных запускаем до подключения к точке
    LittleFS.begin(true);
    m_db.begin();

    // подключение и реакция на подключение или ошибку
    NetworkConnectionService.begin(m_db, m_sets);

    // запускаем сервер после connect, иначе DNS не подхватится
    m_sets.begin();
    m_sets.onBuild(build);
    m_sets.onUpdate(update);
}

void loop() 
{
    m_sets.tick();
    NetworkConnectionService.update();
}
