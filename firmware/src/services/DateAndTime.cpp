#include "DateAndTime.h"
#include "webserver/SettingsWebApp.h"
#include <GyverNTP.h>

namespace service
{
    DB_KEYS(ntp, gmt, host);

    void DateAndTimeClass::begin()
    {
        Serial.println("DateAndTimeClass: begin");

        // set default and get current settings
        webserver::Settings.data().init(ntp::gmt,  0);
        webserver::Settings.data().init(ntp::host, "pool.ntp.org");
        NTP.setGMT(webserver::Settings.data()[ntp::gmt]);
        NTP.setHost(webserver::Settings.data()[ntp::host]);
        NTP.begin();
    }

    void DateAndTimeClass::update()
    {
        NTP.tick();
    }

    void DateAndTimeClass::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Date & time");
        if (b.Input(ntp::gmt, "Time zone")) NTP.setGMT(b.build.value);
        if (b.Input(ntp::host, "NTP server")) NTP.setHost(b.build.value);
        b.LED("synced"_h, "Synced", NTP.synced());
        b.Label("local_time"_h, "Local time", NTP.toString());
    }

    void DateAndTimeClass::settingsUpdate(sets::Updater &u)
    {
        u.update("synced"_h, NTP.synced());
        u.update("local_time"_h, NTP.toString());
    }

    DateAndTimeClass DateAndTime;
}
