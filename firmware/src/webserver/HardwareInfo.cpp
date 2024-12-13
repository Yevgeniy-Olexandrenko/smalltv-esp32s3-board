#include "HardwareInfo.h"
#include "drivers/PowerSource.h"

namespace webserver
{
    void HardwareInfoClass::settingsBuild(sets::Builder &b)
    {
        String moduleChip, moduleMemory;
        String heapUsage, powerSource;
        fillESPModuleInfo(moduleChip, moduleMemory);
        fillHeapUsageInfo(heapUsage);
        fillPowerSourceInfo(powerSource);
        
        sets::Group g(b, "Hardware");
        b.Label("Chip", moduleChip);
        b.Label("Memory", moduleMemory);
        b.Label("heap_usage"_h, "Heap usage", heapUsage);
        b.Label("power_source"_h, "Power source", powerSource);
    }

    void HardwareInfoClass::settingsUpdate(sets::Updater &u)
    {
        String heapUsage, powerSource;
        fillHeapUsageInfo(heapUsage);
        fillPowerSourceInfo(powerSource);

        u.update("heap_usage"_h, heapUsage);
        u.update("power_source"_h, powerSource);
    }

    void HardwareInfoClass::fillESPModuleInfo(String &moduleChip, String &moduleMemory)
    {
        if (m_moduleChip.isEmpty())
        {
            esp_chip_info_t chip_info;
            esp_chip_info(&chip_info);

            m_moduleChip = String(ESP.getChipModel()) + " / " + String(ESP.getCpuFreqMHz()) + "MHz";
            if (ESP.getChipCores() > 1) m_moduleChip += " / 2 Cores";

            auto flashSize = (ESP.getFlashChipSize() / uint32_t(1024 * 1024));
            auto flashEmbedded = (chip_info.features & CHIP_FEATURE_EMB_FLASH);
            m_moduleMemory = "FLASH " + String(flashSize) + "MB " + (flashEmbedded ? "emb" : "ext");

            if (psramFound())
            {
                auto psramSize = (esp_spiram_get_size() / uint32_t(1024 * 1024));
                auto psramEmbedded = (chip_info.features & CHIP_FEATURE_EMB_PSRAM);
                m_moduleMemory += " / PSRAM " + String(psramSize) + "MB " + (psramEmbedded ? "emb" : "ext");
            }
        }
        moduleChip = m_moduleChip;
        moduleMemory = m_moduleMemory;
    }

    void HardwareInfoClass::fillHeapUsageInfo(String &heapUsage)
    {
        auto ramUsed = float(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize() * 100.f;
        heapUsage = "RAM " + String(ramUsed, 1) + "%";

        if (psramFound())
        {
            auto psramUsed = float(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize() * 100.f;
            heapUsage += " / PSRAM " + String(psramUsed, 1) + "%";
        }
    }

    void HardwareInfoClass::fillPowerSourceInfo(String &powerSource)
    {
        switch(driver::PowerSource.getType())
        {
            case driver::PowerSourceType::Unknown:
            {    powerSource = String(driver::PowerSource.getInputVoltage()) + "V";
            }    break;

            case driver::PowerSourceType::Battery:
            {   auto batVolts = driver::PowerSource.getInputVoltage();
                auto batLevel = driver::PowerSource.getPatteryLevelPercents();
                powerSource = "BAT " + String(batVolts) + "V / " + String(batLevel) + "%";
            }   break;

            case driver::PowerSourceType::USB:
            {   auto usbVolts = driver::PowerSource.getInputVoltage();
                powerSource = "USB " + String(usbVolts) + "V";
            }   break;
        }
    }

    HardwareInfoClass HardwareInfo;
}
