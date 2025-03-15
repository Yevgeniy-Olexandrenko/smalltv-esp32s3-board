#include "Drivers.h"
#include "shared/settings/Settings.h"
#include "settings.h"

namespace drivers
{
    using namespace driver;

    void begin()
    {
        // power source
        #ifndef NO_VINSENSE
        powerSource.begin();
        #endif

        // storage
        #ifndef NO_SDCARD
        settings::data().init(db::storage_type, int(Storage::Type::Auto));
        #else
        settings::data().init(db::storage_type, int(Storage::Type::Flash));
        #endif
        auto storageType = Storage::Type(int(settings::data()[db::storage_type]));
        storage.begin(storageType);

        // display
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