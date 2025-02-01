#include "HardwareInfo.h"
#include "drivers/onboard/PowerSource.h"
#include "services/network-connection/NetworkConnection.h"

namespace webserver
{
    void HardwareInfoClass::settingsBuild(sets::Builder &b)
    {
        String moduleChip, moduleMemory;
        String heapUsage, powerSource, wifiSignal;
        fillESPModuleInfo(moduleChip, moduleMemory);
        fillHeapUsageInfo(heapUsage);
        fillPowerSourceInfo(powerSource);
        fillWiFiSignalInfo(wifiSignal);
        
        sets::Group g(b, "Hardware");
        b.Label("Chip", moduleChip);
        b.Label("Memory", moduleMemory);
        b.Label("heap_usage"_h, "Heap usage", heapUsage);
        b.Label("power_source"_h, "Power source", powerSource);
        b.Label("wifi_signal"_h, "WiFi signal", wifiSignal);
    }

    void HardwareInfoClass::settingsUpdate(sets::Updater &u)
    {
        String heapUsage, powerSource, wifiSignal;
        fillHeapUsageInfo(heapUsage);
        fillPowerSourceInfo(powerSource);
        fillWiFiSignalInfo(wifiSignal);

        u.update("heap_usage"_h, heapUsage);
        u.update("power_source"_h, powerSource);
        u.update("wifi_signal"_h, wifiSignal);
    }

    void HardwareInfoClass::fillESPModuleInfo(String &moduleChip, String &moduleMemory)
    {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        moduleChip = String(ESP.getChipModel()) + " / " + String(ESP.getCpuFreqMHz()) + "MHz";
        if (ESP.getChipCores() > 1) moduleChip += " / 2 Cores";

        auto flashSize = (ESP.getFlashChipSize() / uint32_t(1024 * 1024));
        auto flashEmbedded = (chip_info.features & CHIP_FEATURE_EMB_FLASH);
        moduleMemory = "FLASH " + String(flashSize) + "MB " + (flashEmbedded ? "emb" : "ext");

        if (psramFound())
        {
            auto psramSize = (esp_spiram_get_size() / uint32_t(1024 * 1024));
            auto psramEmbedded = (chip_info.features & CHIP_FEATURE_EMB_PSRAM);
            moduleMemory += " / PSRAM " + String(psramSize) + "MB " + (psramEmbedded ? "emb" : "ext");
        }
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
        auto voltage = driver::powerSource.getInputVoltage();
        switch(driver::powerSource.getType())
        {
            case driver::PowerSource::Type::Unknown:
                powerSource = String(voltage) + "V";
                break;

            case driver::PowerSource::Type::Battery:
            {   auto level = driver::powerSource.getBatteryLevelPercents();
                powerSource = "BAT " + String(voltage) + "V / " + String(level) + "%";
            }   break;

            case driver::PowerSource::Type::USB:
                powerSource = "USB " + String(voltage) + "V";
                break;
        }
    }

    void HardwareInfoClass::fillWiFiSignalInfo(String &wifiSignal)
    {
        wifiSignal = String(service::networkConnection.getSignalRSSI()) + "dBm";
        switch (service::networkConnection.getSignalStrength())
        {
            case service::NetworkConnection::Signal::Excellent: wifiSignal += " Excellent"; break;
            case service::NetworkConnection::Signal::Good: wifiSignal += " Good"; break;
            case service::NetworkConnection::Signal::Fair: wifiSignal += " Fair"; break;
            case service::NetworkConnection::Signal::Bad: wifiSignal += " Bad"; break;
        }
    }

    HardwareInfoClass HardwareInfo;
}
