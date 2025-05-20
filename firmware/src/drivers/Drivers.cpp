#include "Drivers.h"
#include "core/settings/Settings.h"
#include "settings.h"

namespace drivers
{
    using namespace driver;

    void begin()
    {
        // init power source
        #ifndef NO_VINSENSE
        powerSource.begin();
        #endif

        // init storage
        #if !defined(NO_SDCARD) && !defined(NO_FLASH)
        auto storageType = Storage::Type::Auto;
        #elif !defined(NO_SDCARD) && defined(NO_FLASH)
        auto storageType = Storage::Type::SDCard;
        #elif defined(NO_SDCARD) && !defined(NO_FLASH)
        auto storageType = Storage::Type::Flash;
        #else
        auto storageType = Storage::Type::None;
        #endif
        settings::data().init(db::storage_type, int(storageType));
        storageType = Storage::Type(int(settings::data()[db::storage_type]));
        storage.begin(storageType);

        // init display
        #ifndef NO_VIDEO
        settings::data().init(db::lcd_brightness, 50);
        auto brightness = float(settings::data()[db::lcd_brightness]) * 0.01f;
        display.begin(brightness);
        #endif

        // start USB MSC on reboot 
        #ifndef NO_USBMSC
        settings::data().init(db::reboot_to_msc, false);
        if (settings::data()[db::reboot_to_msc])
        {
            settings::data()[db::reboot_to_msc] = false;
            storage.startMSC();
        }
        #endif  
    }
}
