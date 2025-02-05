#include "GeoLocation.h"

namespace service
{
    void GeoLocation::begin()
    {
        // TODO
    }

    void GeoLocation::update()
    {
        // TODO
    }

    void GeoLocation::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Geolocation");
        b.Label("TODO");
    }

    void GeoLocation::settingsUpdate(sets::Updater &u)
    {
        // TODO
    }

    GeoLocation geoLocation;
}