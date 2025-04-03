#include "Sets.h"
#include "drivers/Drivers.h"
#include "services/Services.h"
#include "settings.h"

namespace service::settings_webapp
{
    const char casingColors[] = "White;Black;Yellow;Orange;Pink;Silver;Gold";
    const char themeColors [] = "Aqua;Blue;Gray;Green;Mint;Orange;Pink;Red;Violet";
    const sets::Colors themeColorsRGB[] = 
    { 
        sets::Colors::Aqua,
        sets::Colors::Blue,
        sets::Colors::Gray,
        sets::Colors::Green,
        sets::Colors::Mint,
        sets::Colors::Orange,
        sets::Colors::Pink,
        sets::Colors::Red,
        sets::Colors::Violet
    };

    void Sets::begin()
    {
        settings::data().init(db::color_casing, 0);
        settings::data().init(db::color_theme,  0);
    }

    void Sets::settingsBuild(sets::Builder &b)
    {
        colorsSettingsBuild(b);
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

    String Sets::getCasingColor() const
    {
        int index = settings::data()[db::color_casing];
        return Text(casingColors).getSub(index, ';').toString();
    }

    sets::Colors Sets::getThemeColor() const
    {
        int index = settings::data()[db::color_theme];
        return themeColorsRGB[index];
    }

    ////////////////////////////////////////////////////////////////////////////

    void Sets::colorsSettingsBuild(sets::Builder &b)
    {
        sets::Row r(b, "", sets::DivType::Line);
        b.Select(db::color_casing, "Casing", casingColors);
        if (b.Select(db::color_theme, "Theme", themeColors))
        {
            auto color = getThemeColor();
            settings::sets().config.theme = color;
            b.reload();
        }
    }

    void Sets::storageSettingsBuild(sets::Builder &b)
    {
        b.beginGuest();
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
        b.endGuest();
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
