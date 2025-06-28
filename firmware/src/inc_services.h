#pragma once

#include "services/DateTime.h"
#include "services/GeoLocation.h"
#include "services/WiFiConnection.h"
#include "services/SettingsWebApp.h"
#include "services/WeatherForecast.h"

#ifndef NO_AUDIO
#include "services/AudioPlayer.h"
#endif

#include "settings.h"
#include "secrets.h"

namespace services
{
    void begin()
    {
        // init API keys
        settings::apikey(db::apikey_google, APIKEY_GOOGLE);
        settings::apikey(db::apikey_openweather, APIKEY_OPENWEATHER);

        // required services
        service::dateTime.begin();
        service::geoLocation.begin();
        service::wifiConnection.begin();
        service::settingsWebApp.begin();
        service::weatherForecast.begin();

        // optional services
        #ifndef NO_AUDIO
        service::audioPlayer.begin();
        #endif
    }
}
