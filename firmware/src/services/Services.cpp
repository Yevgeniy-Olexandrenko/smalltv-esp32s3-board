#include "Services.h"
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
        service::weatherForecast.begin();
        service::settingsWebApp.begin();

        // optional services
        #ifndef NO_AUDIO
        service::audioPlayer.begin();
        #endif
    }

    void update()
    {
        service::geoLocation.update();
        service::weatherForecast.update();
    }
}
