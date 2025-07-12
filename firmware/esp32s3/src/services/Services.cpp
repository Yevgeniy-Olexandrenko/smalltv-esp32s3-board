#include "Services.h"
#include "core/settings/Settings.h"
#include "firmware/secrets.h"
#include "settings.h"

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

    void update()
    {
        // TODO
    }

    void onConnectedToWiFi()
    {
        log_i("on connected to wifi");

        // TODO
    }

    void onDisconnectedFromWiFi()
    {
        log_i("on disconnected from wifi");
        
        // TODO
    }
}
