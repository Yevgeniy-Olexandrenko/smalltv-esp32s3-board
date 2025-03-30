#pragma once

#include "audio_player/AudioPlayer.h"
#include "date_and_time/DateAndTime.h"
#include "geo_location/GeoLocation.h"
#include "settings_webapp/SettingsWebApp.h"
#include "weather_forecast/WeatherForecast.h"
#include "wifi_connection/WiFiConnection.h"

namespace services
{
    void begin();
    void update();
}
