#pragma once

// drivers
#include "drivers/PowerSource.h"
#include "drivers/Storage.h"
#include "drivers/Display.h"
#include "drivers/SelfReboot.h"
#include "drivers/StatusLED.h"

// commons
#include "settings/Settings.h"
#include "firmware/secrets.h"

namespace drivers
{
    void begin()
    {
        // init power source
        #ifndef NO_VINSENSE
        driver::powerSource.begin();
        #endif

        // init storage
        #if !defined(NO_SDCARD) && !defined(NO_FLASH)
        auto storageType = driver::Storage::Type::Auto;
        #elif !defined(NO_SDCARD) && defined(NO_FLASH)
        auto storageType = driver::Storage::Type::SDCard;
        #elif defined(NO_SDCARD) && !defined(NO_FLASH)
        auto storageType = driver::Storage::Type::Flash;
        #else
        auto storageType = driver::Storage::Type::None;
        #endif
        Settings::data().init(storage::type, int(storageType));
        storageType = driver::Storage::Type(int(Settings::data()[storage::type]));
        driver::storage.begin(storageType);

        // init display
        #ifndef NO_VIDEO
        Settings::data().init(display::brightness, 50);
        auto brightness = float(Settings::data()[display::brightness]) * 0.01f;
        driver::display.begin(brightness);
        #endif

        // start USB MSC on reboot 
        #ifndef NO_USBMSC
        Settings::data().init(reboot::to_msc, false);
        if (Settings::data()[reboot::to_msc])
        {
            Settings::data()[reboot::to_msc] = false;
            driver::storage.startMSC();
        }
        #endif  
    }
}

namespace apps
{
    void begin()
    {
        #ifndef NO_VIDEO
        driver::display.fillScreen(TFT_NAVY);
        #endif    
        
        // TODO
    }
}
