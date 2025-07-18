#include "Services.h"
#include "settings/Settings.h"
#include "firmware/secrets.h"
#include "settings.h"

namespace services
{
    void begin()
    {
        // init API keys
        Settings::keys().init(apikey::google, APIKEY_GOOGLE);
        Settings::keys().init(apikey::openweather, APIKEY_OPENWEATHER);

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
}
