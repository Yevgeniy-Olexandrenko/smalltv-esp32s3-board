#include "Services.h"
#include "shared/settings/Settings.h"
#include "settings.h"

namespace services
{
    void begin()
    {
        // set default settings
        settings::data().init(db::audio_volume, 50);

        // get current settings
        auto audioVolume = float(settings::data()[db::audio_volume]) * 0.01f;

        // begin services with current settings
        // service::networkConnection.begin();
        // service::geoLocation.begin();
        // service::dateAndTime.begin();
        // service::weatherForecast.begin();
        // service::settingsWebApp.begin();
        service::audioPlayer.begin(audioVolume);
    }

    void update()
    {
        // service::networkConnection.update();
        // service::geoLocation.update();
        // service::dateAndTime.update();
        // service::weatherForecast.update();
    }
}
