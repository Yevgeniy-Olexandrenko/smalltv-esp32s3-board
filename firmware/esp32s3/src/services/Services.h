#pragma once

#include "services/DateTime.h"
#include "services/GeoLocation.h"
#include "services/WiFiConnection.h"
#include "services/SettingsWebApp.h"
#include "services/WeatherForecast.h"
#include "services/AudioPlayer.h"

namespace services
{
    void begin();
    void update();

    void onConnectedToWiFi();
    void onDisconnectedFromWiFi();
}
