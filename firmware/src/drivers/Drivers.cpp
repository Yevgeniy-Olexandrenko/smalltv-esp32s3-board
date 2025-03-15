#include "Drivers.h"
#include "shared/settings/Settings.h"
#include "settings.h"

namespace drivers
{
    using namespace driver;

    // default storage type
    const Storage::Type getDefaultStorageType()
    {
        #ifndef NO_SDCARD
        return Storage::Type::Auto;
        #else
        return Storage::Type::Flash;
        #endif
    }

    void begin()
    {
        // set default settings
        settings::data().init(db::storage_type, int(getDefaultStorageType()));
        settings::data().init(db::lcd_brightness, 50);
        settings::data().init(db::reboot_to_msc, false);

        // get current settings
        auto storageType = Storage::Type(int(settings::data()[db::storage_type]));
        auto lcdBrightness = float(settings::data()[db::lcd_brightness]) * 0.01f;

        // begin drivers with current settings
        powerSource.begin();
        storage.begin(storageType);
        display.begin(lcdBrightness);

        // on reboot
        if (settings::data()[db::reboot_to_msc])
        {
            settings::data()[db::reboot_to_msc] = false;
            storage.startMSC();
        }
    }
}