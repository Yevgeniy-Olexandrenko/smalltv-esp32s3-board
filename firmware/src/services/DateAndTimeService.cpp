#include "DateAndTimeService.h"
#include "SettingsService.h"
#include <GyverNTP.h>

DB_KEYS(ntp, gmt, host);

void DateAndTimeServiceClass::begin()
{
    Serial.println("DateAndTimeServiceClass: begin");

    // set default and get current settings
    SettingsService.data().init(ntp::gmt,  0);
    SettingsService.data().init(ntp::host, "pool.ntp.org");
    NTP.setGMT(SettingsService.data()[ntp::gmt]);
    NTP.setHost(SettingsService.data()[ntp::host]);
    NTP.begin();
}

void DateAndTimeServiceClass::update()
{
    NTP.tick();
}

void DateAndTimeServiceClass::settingsBuild(sets::Builder &b)
{
    sets::Group g(b, "Time");
    b.Input(ntp::gmt, "Time zone");
    b.Input(ntp::host, "NTP server");
    b.LED("synced"_h, "Synced", NTP.synced());
    b.Label("local_time"_h, "Local time", NTP.timeToString());

    if (b.build.isAction()) 
    {
        switch (b.build.id) 
        {
            case ntp::gmt: NTP.setGMT(b.build.value); break;
            case ntp::host: NTP.setHost(b.build.value); break;
        }
    }
}

void DateAndTimeServiceClass::settingsUpdate(sets::Updater &u)
{
    u.update("synced"_h, NTP.synced());
    u.update("local_time"_h, NTP.timeToString());
}

DateAndTimeServiceClass DateAndTimeService;
