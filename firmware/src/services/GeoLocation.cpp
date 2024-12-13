#include "GeoLocation.h"
#include "webserver/SettingsWebApp.h"

namespace service
{
    void GeoLocationClass::begin()
    {
        // TODO
    }

    void GeoLocationClass::update()
    {
        // TODO
    }

    void GeoLocationClass::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Geolocation");
        b.Label("TODO");
    }

    void GeoLocationClass::settingsUpdate(sets::Updater &u)
    {
        // TODO
    }

    GeoLocationClass GeoLocation;
}