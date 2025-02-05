#include "Services.h"

namespace services
{
    void begin()
    {
        service::networkConnection.begin();
        service::geoLocation.begin();
        service::dateAndTime.begin();
        service::weatherForecast.begin();
        service::settingsWebApp.begin();
    }
}
