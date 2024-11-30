#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <WiFiConnector.h>

#include "services/SettingsService.h"
#include "services/NetworkConnectionService.h"
#include "services/DateAndTimeService.h"
#include "services/GeoLocationService.h"
#include "services/WeatherService.h"

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

    NetworkConnectionService.begin();
    GeoLocationService.begin();
    DateAndTimeService.begin();
    WeatherService.begin();

    SettingsService.sets().onBuild(build);
    SettingsService.sets().onUpdate(update);
}

void loop() 
{
    SettingsService.update();
    NetworkConnectionService.update();

    // TODO
}
