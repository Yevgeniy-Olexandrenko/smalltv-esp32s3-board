#pragma once

// drivers
#include "drivers/PowerSource.h"
#include "drivers/Storage.h"
#include "drivers/Display.h"
#include "drivers/SelfReboot.h"
#include "drivers/StatusLED.h"

// services
#include "services/DateTime.h"
#include "services/GeoLocation.h"
#include "services/WiFiConnection.h"
#include "services/SettingsWebApp.h"
#include "services/WeatherForecast.h"
#include "services/AudioPlayer.h"

// commons
#include "core/settings/Settings.h"
#include "firmware/secrets.h"
#include "settings.h"

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
        settings::data().init(db::storage_type, int(storageType));
        storageType = driver::Storage::Type(int(settings::data()[db::storage_type]));
        driver::storage.begin(storageType);

        // init display
        #ifndef NO_VIDEO
        settings::data().init(db::lcd_brightness, 50);
        auto brightness = float(settings::data()[db::lcd_brightness]) * 0.01f;
        driver::display.begin(brightness);
        #endif

        // start USB MSC on reboot 
        #ifndef NO_USBMSC
        settings::data().init(db::reboot_to_msc, false);
        if (settings::data()[db::reboot_to_msc])
        {
            settings::data()[db::reboot_to_msc] = false;
            driver::storage.startMSC();
        }
        #endif  
    }
}

namespace services
{
    void begin()
    {
        // init API keys
        settings::apikey(db::apikey_google, APIKEY_GOOGLE);
        settings::apikey(db::apikey_openweather, APIKEY_OPENWEATHER);

        // required services
        service::dateTime.begin();
        service::geoLocation.begin();
        service::wifiConnection.begin();
        service::settingsWebApp.begin();
        service::weatherForecast.begin();

        // optional services
        #ifndef NO_AUDIO
        service::audioPlayer.begin();
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
