#include "Sets.h"
#include "drivers/Drivers.h"
#include "services/Services.h"
#include "settings.h"

namespace service_settings_webapp_impl
{
    void Sets::settingsBuild(sets::Builder &b)
    {
        service::wifiConnection.getUI().settingsBuild(b);
        storageSettingsBuild(b);
        service::geoLocation.settingsBuild(b);
        service::dateAndTime.settingsBuild(b);
        service::weatherForecast.settingsBuild(b);
    }

    void Sets::settingsUpdate(sets::Updater &u)
    {
        service::wifiConnection.getUI().settingsUpdate(u);
        storageSettingsUpdate(u);
        service::geoLocation.settingsUpdate(u);
        service::dateAndTime.settingsUpdate(u);
        service::weatherForecast.settingsUpdate(u);
    }

    ////////////////////////////////////////////////////////////////////////////

    void Sets::storageSettingsBuild(sets::Builder &b)
    {
        if (m_typeRollback < 0)
            m_typeRollback = settings::data()[db::storage_type];

        {
            String choice = "None;Embedded Flash";
            #ifndef NO_SDCARD
            choice += ";External SD Card;Auto";
            #endif

            sets::Group g(b, "💾 Storage");
            if (b.Select(db::storage_type, "Type", choice))
            {
                m_typeChanged = (settings::data()[db::storage_type] != m_typeRollback);
            }

            if (driver::storage.getType() != driver::Storage::Type::None)
            {
                String specs;
                fillStorageSpecs(specs);
                b.Label("Specs", specs);

                #ifndef NO_USBMSC
                if (b.Button("Connect to PC"))
                {
                    service::settingsWebApp.requestReboot([&](bool reboot)
                    {
                        if (reboot)
                            settings::data()[db::reboot_to_msc] = true;
                    });
                }
                #endif
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
        #ifndef NO_SDCARD
        if (driver::storage.getType() == driver::Storage::Type::SDCard)
        {
            const auto size = driver::storage.getFS().partitionSize() / (1000.f * 1000.f);
            const auto& sdcard = static_cast<driver::SDCard&>(driver::storage.getFS()); 
            specs = (sdcard.getCardType() == driver::SDCard::Type::SDHC ? "SDHC" : "SDSC");
            specs += " " + (size > 1000 ? String(size / 1000.f, 0) + "GB" : String(size, 0) + "MB");
            switch(sdcard.getCardInterface())
            {
                case driver::SDCard::Interface::SPI: specs += " / SPI"; break;
                case driver::SDCard::Interface::SDIO1 : specs += " / SDIO-1B"; break;
                case driver::SDCard::Interface::SDIO4 : specs += " / SDIO-4B"; break;
            }
        }
        else
        #endif
        #ifndef NO_FLASH
        if (driver::storage.getType() == driver::Storage::Type::Flash)
        {
            const auto size = driver::storage.getFS().partitionSize() / (1024.f * 1024.f);
            specs = "FLASH " + String(size, 0) + "MB / SPI";
            
        }
        #endif
    }

    ////////////////////////////////////////////////////////////////////////////

}
