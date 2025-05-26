#pragma once

#include "date_time/DateTime.h"
#include "geo_location/GeoLocation.h"
#include "wifi_connection/WiFiConnection.h"
#include "weather_forecast/WeatherForecast.h"
#include "settings_webapp/SettingsWebApp.h"

#ifndef NO_AUDIO
#include "audio_player/AudioPlayer.h"
#endif

namespace services
{
    void begin();
    void update();
}
