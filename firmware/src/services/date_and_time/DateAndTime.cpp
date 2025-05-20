#include "DateAndTime.h"
#include "core/settings/Settings.h"
#include <GyverNTP.h>

namespace service
{
    DB_KEYS(ntp, gmt, host);

    void DateAndTime::begin()
    {
        Serial.println("DateAndTimeClass: begin");

        // set default and get current settings
        settings::data().init(ntp::gmt,  0);
        settings::data().init(ntp::host, "pool.ntp.org");
        NTP.setGMT(settings::data()[ntp::gmt]);
        NTP.setHost(settings::data()[ntp::host]);
        NTP.begin();
    }

    void DateAndTime::update()
    {
        NTP.tick();
    }

    void DateAndTime::settingsBuild(sets::Builder &b)
    {
        b.beginGuest();
        sets::Group g(b, "Date & time");
        if (b.Input(ntp::gmt, "Time zone")) NTP.setGMT(b.build.value);
        if (b.Input(ntp::host, "NTP server")) NTP.setHost(b.build.value);
        b.LED("synced"_h, "Synced", NTP.synced());
        b.Label("local_time"_h, "Local time", NTP.toString());
        b.endGuest();
    }

    void DateAndTime::settingsUpdate(sets::Updater &u)
    {
        u.update("synced"_h, NTP.synced());
        u.update("local_time"_h, NTP.toString());
    }

    DateAndTime dateAndTime;
}
