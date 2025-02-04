#include "Sets.h"
#include "services/network-connection/NetworkConnection.h"
#include "services/date-and-time/DateAndTime.h"
#include "services/weather-forecast/WeatherForecast.h"
#include "services/settings-webapp/SettingsWebApp.h"
#include "drivers/storage/SDCard.h"
#include "drivers/storage/Flash.h"

namespace service_settings_webapp_impl
{
    void Sets::settingsBuild(sets::Builder &b)
    {
        service::networkConnection.settingsBuild(b);
        service::dateAndTime.settingsBuild(b);
        service::weatherForecast.settingsBuild(b);
        storageSettingsBuild(b);
    }

    void Sets::settingsUpdate(sets::Updater &u)
    {
        service::networkConnection.settingsUpdate(u);
        service::dateAndTime.settingsUpdate(u);
        service::weatherForecast.settingsUpdate(u);
        storageSettingsUpdate(u);
    }

    ////////////////////////////////////////////////////////////////////////////

    DB_KEYS(storage, type);

    driver::Storage::Type Sets::getStorageType() const
    {
        settings::data().init(storage::type, int(driver::Storage::Type::Auto));
        return driver::Storage::Type(int(settings::data()[storage::type]));
    }

    ////////////////////////////////////////////////////////////////////////////

    void Sets::storageSettingsBuild(sets::Builder &b)
    {
        if (m_typeRollback < 0)
            m_typeRollback = settings::data()[storage::type];

        sets::Group g(b, "Storage");
        if (b.Select(storage::type, "Type", "None;Embedded Flash;External SD Card;Auto"))
        {
            m_typeChanged = (settings::data()[storage::type] != m_typeRollback);
        }

        if (driver::storage.getType() != driver::Storage::Type::None)
        {
            String specs;
            fillStorageSpecs(specs);
            b.Label("Specs", specs);

            if (b.Button("Connect to PC"))
            {
                driver::storage.startMSC();
                b.reload();
                return;
            }
        }

        if (b.Confirm("confirm"_h, "Changing the storage type requires a reboot"))
        {
            if (b.build.value.toBool())
                service::settingsWebApp.requestDeviceReboot();
            else
                settings::data()[storage::type] = m_typeRollback;
        }
    }

    void Sets::storageSettingsUpdate(sets::Updater &u)
    {
        if (m_typeChanged)
        {
            u.update("confirm"_h);
            m_typeChanged = false;
        }
    }

    void Sets::fillStorageSpecs(String &specs) const
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

    ////////////////////////////////////////////////////////////////////////////

}
