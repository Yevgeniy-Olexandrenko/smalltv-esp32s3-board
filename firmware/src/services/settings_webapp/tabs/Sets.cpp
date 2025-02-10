#include "Sets.h"
#include "drivers/Drivers.h"
#include "services/Services.h"
#include "settings.h"

namespace service_settings_webapp_impl
{
    void Sets::settingsBuild(sets::Builder &b)
    {
        service::networkConnection.settingsBuild(b);
        service::geoLocation.settingsBuild(b);
        service::dateAndTime.settingsBuild(b);
        service::weatherForecast.settingsBuild(b);
        storageSettingsBuild(b);
    }

    void Sets::settingsUpdate(sets::Updater &u)
    {
        service::networkConnection.settingsUpdate(u);
        service::geoLocation.settingsUpdate(u);
        service::dateAndTime.settingsUpdate(u);
        service::weatherForecast.settingsUpdate(u);
        storageSettingsUpdate(u);
    }

    ////////////////////////////////////////////////////////////////////////////

    void Sets::storageSettingsBuild(sets::Builder &b)
    {
        if (m_typeRollback < 0)
            m_typeRollback = settings::data()[db::storage_type];

        {
            String choice = "None;Embedded Flash";
            if (hardware::hasSDCard()) choice += ";External SD Card;Auto";

            sets::Group g(b, "Storage");
            if (b.Select(db::storage_type, "Type", choice))
            {
                m_typeChanged = (settings::data()[db::storage_type] != m_typeRollback);
            }

            if (driver::storage.getType() != driver::Storage::Type::None)
            {
                String specs;
                fillStorageSpecs(specs);
                b.Label("Specs", specs);

                if (hardware::hasUsbMSC() && b.Button("Connect to PC"))
                {
                    service::settingsWebApp.requestReboot([&](bool reboot)
                    {
                        if (reboot)
                            settings::data()[db::reboot_to_msc] = true;
                    });
                }
            }
        }
    }

    void Sets::storageSettingsUpdate(sets::Updater &u)
    {
        if (m_typeChanged)
        {
            m_typeChanged = false;
            service::settingsWebApp.requestReboot([&](bool reboot)
            {
                if (!reboot)
                    settings::data()[db::storage_type] = m_typeRollback;
            });
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
