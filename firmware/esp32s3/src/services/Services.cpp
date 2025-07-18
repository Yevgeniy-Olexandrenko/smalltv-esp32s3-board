#include "Services.h"
#include "settings/Settings.h"
#include "firmware/secrets.h"
#include "settings.h"

namespace services
{
    void begin()
    {
        // init API keys
        Settings::beginApiKeys();
        Settings::data().init(apikey::google, APIKEY_GOOGLE);
        Settings::data().init(apikey::openweather, APIKEY_OPENWEATHER);
        Settings::endApiKeys();

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
