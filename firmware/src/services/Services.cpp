#include "Services.h"
#include "shared/settings/Settings.h"
#include "settings.h"

namespace services
{
    void begin()
    {
        // set default settings
        settings::data().init(db::audio_volume, 100);

        // get current settings
        auto audioVolume = float(settings::data()[db::audio_volume]) / 200;

        // begin services with current settings
        service::networkConnection.begin();
        service::geoLocation.begin();
        service::dateAndTime.begin();
        service::weatherForecast.begin();
        service::settingsWebApp.begin();
        service::audioPlayer.begin(audioVolume);
    }
}
