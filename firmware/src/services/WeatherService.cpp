#include "WeatherService.h"

void WeatherServiceClass::begin()
{
    // TODO
}

void WeatherServiceClass::update()
{
    // TODO
}

void WeatherServiceClass::settingsBuild(sets::Builder &b)
{
    sets::Group g(b, "Weather");
    b.Label("TODO");
}

void WeatherServiceClass::settingsUpdate(sets::Updater &u)
{
    // TODO
}

WeatherServiceClass WeatherService;
