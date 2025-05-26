#include "Services.h"
#include "settings.h"

namespace services
{
    void begin()
    {
        service::dateTime.begin();
        service::geoLocation.begin();
        service::wifiConnection.begin();
        service::weatherForecast.begin();
        service::settingsWebApp.begin();

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
