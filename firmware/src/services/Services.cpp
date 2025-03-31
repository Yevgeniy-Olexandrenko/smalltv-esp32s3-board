#include "Services.h"
#include "shared/settings/Settings.h"
#include "settings.h"

namespace services
{
    void begin()
    {
        service::wifiConnection.begin();
        service::geoLocation.begin();
        service::dateAndTime.begin();
        service::weatherForecast.begin();
        service::settingsWebApp.begin();

        #ifndef NO_AUDIO
        service::audioPlayer.begin();
        #endif
    }

    void update()
    {
        service::geoLocation.update();
        service::dateAndTime.update();
        service::weatherForecast.update();
    }
}
