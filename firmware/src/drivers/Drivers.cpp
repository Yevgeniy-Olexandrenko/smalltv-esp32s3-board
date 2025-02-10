#include "Drivers.h"
#include "shared/settings/Settings.h"
#include "settings.h"

namespace drivers
{
    using namespace driver;

    // default storage type
    const Storage::Type getDefaultStorageType()
    {
        return (hardware::hasSDCard() ? Storage::Type::Auto : Storage::Type::Flash);
    }

    void begin()
    {
        // set default settings
        settings::data().init(db::storage_type, int(getDefaultStorageType()));
        settings::data().init(db::lcd_brightness, LCDBacklight::RANGE / 2);
        settings::data().init(db::audio_volume, 100);
        // TODO
        settings::data().init(db::reboot_to_msc, false);

        // get current settings
        auto storageType = Storage::Type(int(settings::data()[db::storage_type]));
        auto lcdBrightness = LCDBacklight::Brightness(settings::data()[db::lcd_brightness]);
        auto audioVolume = int(settings::data()[db::audio_volume]);

        // begin drivers with current settings
        powerSource.begin();
        storage.begin(storageType);
        lcdBacklight.begin(lcdBrightness);

        // on reboot
        if (settings::data()[db::reboot_to_msc])
        {
            settings::data()[db::reboot_to_msc] = false;
            storage.startMSC();
        }
    }
}