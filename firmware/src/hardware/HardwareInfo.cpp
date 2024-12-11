#include "HardwareInfo.h"

void HardwareInfoClass::settingsBuild(sets::Builder &b)
{
    if (m_chip.isEmpty())
    {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        m_chip = String(ESP.getChipModel()) + " / " + String(ESP.getCpuFreqMHz()) + "MHz";
        if (ESP.getChipCores() > 1) m_chip += " / 2 Cores";

        auto flashSize = (ESP.getFlashChipSize() / uint32_t(1024 * 1024));
        auto flashEmbedded = (chip_info.features & CHIP_FEATURE_EMB_FLASH);
        m_memory = "FLASH " + String(flashSize) + "MB " + (flashEmbedded ? "emb" : "ext");

        if (psramFound())
        {
            auto psramSize = (esp_spiram_get_size() / uint32_t(1024 * 1024));
            auto psramEmbedded = (chip_info.features & CHIP_FEATURE_EMB_PSRAM);
            m_memory += " / PSRAM " + String(psramSize) + "MB " + (psramEmbedded ? "emb" : "ext");
        }
    }

    auto ramUsed = float(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize() * 100.f;
    String ramUsage = "RAM " + String(ramUsed, 1) + "%";
    if (psramFound())
    {
        auto psramUsed = float(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize() * 100.f;
        ramUsage += " / PSRAM " + String(psramUsed, 1) + "%";
    }
    
    sets::Group g(b, "Hardware");
    b.Label("SoC", m_chip);
    b.Label("Memory", m_memory);
    b.Label("Heap usage", ramUsage);
}

void HardwareInfoClass::settingsUpdate(sets::Updater &u)
{
}

HardwareInfoClass HardwareInfo;
