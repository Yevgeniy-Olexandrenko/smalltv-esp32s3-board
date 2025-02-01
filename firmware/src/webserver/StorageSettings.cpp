#include "SettingsWebApp.h"
#include "StorageSettings.h"
#include "drivers/storage/SDCard.h"
#include "drivers/storage/Flash.h"

namespace webserver
{
    DB_KEYS(storage, type);

    void StorageSettingsClass::settingsBuild(sets::Builder &b)
    {
        if (_typeRollback < 0)
            _typeRollback = Settings.data()[storage::type];

        sets::Group g(b, "Storage");
        if (b.Select(storage::type, "Type", "None;Embedded Flash;External SD Card;Auto"))
        {
            _typeChanged = (Settings.data()[storage::type] != _typeRollback);
        }

        if (driver::storage.getType() != driver::Storage::Type::None)
        {
            String specs;
            fillStorageSpecs(specs);
            b.Label("Specs", specs);

            bool msc_run = driver::storage.isMSCRunning();
            if (b.Switch("msc_run"_h, "Connect to PC", &msc_run) && msc_run)
            {
                driver::storage.startMSC();
            }
        }

        if (b.Confirm("confirm"_h, "Changing the storage type requires a reboot"))
        {
            if (b.build.value.toBool())
                SettingsWebApp.requestDeviceRestart();
            else
                Settings.data()[storage::type] = _typeRollback;
        }
    }

    void StorageSettingsClass::settingsUpdate(sets::Updater &u)
    {
        u.update("msc_run"_h, driver::storage.isMSCRunning());
        if (_typeChanged)
        {
            u.update("confirm"_h);
            _typeChanged = false;
        }
    }

    driver::Storage::Type StorageSettingsClass::getStorageType() const
    {
        Settings.data().init(storage::type, int(driver::Storage::Type::Auto));
        return driver::Storage::Type(int(Settings.data()[storage::type]));
    }

    void StorageSettingsClass::fillStorageSpecs(String &specs) const
    {
        if (driver::storage.getType() == driver::Storage::Type::SDCard)
        {
            auto size = driver::sdcard.getPartitionSize() / (1000.f * 1000.f);
            specs = (driver::sdcard.getCardType() == driver::SDCard::Type::SDHC ? "SDHC" : "SDSC");
            specs += " " + (size > 1000 ? String(size / 1000.f, 0) + "GB" : String(size, 0) + "MB");
            switch(driver::sdcard.getCardInterface())
            {
                case driver::SDCard::Interface::SPI: specs += " / SPI"; break;
                case driver::SDCard::Interface::SDIO1 : specs += " / SDIO-1B"; break;
                case driver::SDCard::Interface::SDIO4 : specs += " / SDIO-4B"; break;
            }
        }
        else if (driver::storage.getType() == driver::Storage::Type::Flash)
        {
            auto size = driver::flash.getPartitionSize() / (1024.f * 1024.f);
            specs = "FLASH " + String(size, 0) + "MB / SPI";
            
        }
    }

    StorageSettingsClass StorageSettings;
}