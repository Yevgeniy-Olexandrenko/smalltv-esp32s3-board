#include "Drivers.h"
#include "shared/settings/Settings.h"
#include "settings.h"

namespace drivers
{
    void begin()
    {
        using namespace driver;

        // set default settings
        settings::data().init(db::storage_type, int(Storage::Type::Auto));
        settings::data().init(db::lcd_brightness, LCDBacklight::RANGE / 2);
        settings::data().init(db::audio_volume, 100);

        // get current settings
        auto storageType = Storage::Type(int(settings::data()[db::storage_type]));
        auto lcdBrightness = LCDBacklight::Brightness(settings::data()[db::lcd_brightness]);
        auto audioVolume = int(settings::data()[db::audio_volume]);

        // begin drivers with current settings
        powerSource.begin();
        storage.begin(storageType);
        lcdBacklight.begin(lcdBrightness);
    }
}