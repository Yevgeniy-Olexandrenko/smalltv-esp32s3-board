#include "GeoLocationService.h"

void GeoLocationServiceClass::begin()
{
    // TODO
}

void GeoLocationServiceClass::update()
{
    // TODO
}

void GeoLocationServiceClass::settingsBuild(sets::Builder &b)
{
    sets::Group g(b, "Geolocation");
    b.Label("TODO");
}

void GeoLocationServiceClass::settingsUpdate(sets::Updater &u)
{
    // TODO
}

GeoLocationServiceClass GeoLocationService;
