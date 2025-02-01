#include "Weather.h"
#include "webserver/SettingsWebApp.h"

namespace service
{
    void WeatherClass::begin()
    {
        // TODO
    }

    void WeatherClass::update()
    {
        // TODO
    }

    void WeatherClass::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Weather");
        b.Label("TODO");
    }

    void WeatherClass::settingsUpdate(sets::Updater &u)
    {
        // TODO
    }

    WeatherClass Weather;
}
