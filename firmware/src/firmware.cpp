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

static bool m_restartRequested = false;

void settingsBuild(sets::Builder& b) 
{
    NetworkConnectionService.settingsBuild(b);
    {
        sets::Group g(b, "Settings");
        {
            sets::Menu m(b, "Globals");
            GeoLocationService.settingsBuild(b);
            DateAndTimeService.settingsBuild(b);
            WeatherService.settingsBuild(b);
        }
        {
            sets::Menu m(b, "Applications");
            b.Label("TODO");
        }
    }

    // TODO

    if (b.Button("Restart"))
    {
        m_restartRequested = true;
        SettingsService.data().update();
        b.reload();
    }
}
    
void settingsUpdate(sets::Updater& u)
{
    NetworkConnectionService.settingsUpdate(u);
    GeoLocationService.settingsUpdate(u);
    DateAndTimeService.settingsUpdate(u);
    WeatherService.settingsUpdate(u);

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

    SettingsService.sets().onBuild(settingsBuild);
    SettingsService.sets().onUpdate(settingsUpdate);
}

void loop() 
{
    SettingsService.update();
    NetworkConnectionService.update();
    GeoLocationService.update();
    DateAndTimeService.update();
    WeatherService.update();

    // TODO

    if (m_restartRequested) ESP.restart();
}
