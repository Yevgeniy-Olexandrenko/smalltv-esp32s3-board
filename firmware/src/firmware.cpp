#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <WiFiConnector.h>

#include "services/Settings/SettingsService.h"
#include "services/NetworkConnection/NetworkConnectionService.h"
#include "services/DateAndTime/DateAndTimeService.h"
#include "services/GeoLocation/GeoLocationService.h"
#include "services/Weather/WeatherService.h"

#include "hardware/HardwareInfo.h"

#define ONBOARD_LED GPIO_NUM_0
#define DISPLAY_BACKLIGHT GPIO_NUM_14
#define POWER_SOURCE_VOLTAGE GPIO_NUM_3

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
    HardwareInfo.settingsBuild(b);

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
    delay(1500);

    // turn on the onboard led
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, LOW);

    // turn on the display backlight
    pinMode(DISPLAY_BACKLIGHT, OUTPUT);
    digitalWrite(DISPLAY_BACKLIGHT, HIGH);

    // print power source voltage
    analogSetPinAttenuation(POWER_SOURCE_VOLTAGE, ADC_11db);
    auto voltage = 0.002f * analogReadMilliVolts(POWER_SOURCE_VOLTAGE);
    Serial.print("Power source voltage: ");
    Serial.println(voltage);

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
