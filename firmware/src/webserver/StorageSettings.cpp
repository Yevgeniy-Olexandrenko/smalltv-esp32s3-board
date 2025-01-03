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
            b.Label("specs"_h, "Specs", specs);

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
        String specs;
        fillStorageSpecs(specs);
        u.update("specs"_h, specs);

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
            specs = (driver::sdcard.getCardType() == CARD_SDHC ? "SDHC" : "SDSC");
            specs += " " + (size > 1000 ? String(size / 1000.f, 0) + "Gb" : String(size, 0) + "Mb");
        }
        else if (driver::storage.getType() == driver::Storage::Type::Flash)
        {
            auto size = driver::flash.getPartitionSize() / (1024.f * 1024.f);
            specs = "FLASH " + String(size, 0) + "Mb";
            
        }

        // TODO: this does not update in real time, just after reset
        auto free = float(driver::storage.getFSFreeBytes());
        auto total = float(driver::storage.getFSTotalBytes());
        specs += " / Free " + String(100.f * free / total, 1) + "%";
    }

    StorageSettingsClass StorageSettings;
}