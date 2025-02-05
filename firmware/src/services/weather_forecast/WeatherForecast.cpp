#include "WeatherForecast.h"

namespace service
{
    void WeatherForecast::begin()
    {
        // TODO
    }

    void WeatherForecast::update()
    {
        // TODO
    }

    void WeatherForecast::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Weather");
        b.Label("TODO");
    }

    void WeatherForecast::settingsUpdate(sets::Updater &u)
    {
        // TODO
    }

    WeatherForecast weatherForecast;
}
