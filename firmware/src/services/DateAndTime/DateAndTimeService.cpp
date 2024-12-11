#include "DateAndTimeService.h"
#include "services/Settings/SettingsService.h"
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
    sets::Group g(b, "Date & time");
    if (b.Input(ntp::gmt, "Time zone")) NTP.setGMT(b.build.value);
    if (b.Input(ntp::host, "NTP server")) NTP.setHost(b.build.value);
    b.LED("synced"_h, "Synced", NTP.synced());
    b.Label("local_time"_h, "Local time", NTP.toString());
}

void DateAndTimeServiceClass::settingsUpdate(sets::Updater &u)
{
    u.update("synced"_h, NTP.synced());
    u.update("local_time"_h, NTP.toString());
}

DateAndTimeServiceClass DateAndTimeService;
