#include "Main.h"
#include "drivers/onboard/PowerSource.h"
#include "services/network-connection/NetworkConnection.h"

namespace service_settings_webapp_impl
{
    void Main::settingsBuild(sets::Builder &b)
    {
        String moduleSpecs;
        String ramUsage;
        String psramUsage;
        String powerSource;
        String wifiSignal;

        fillESPModuleInfo(moduleSpecs);
        fillRAMUsageInfo(ramUsage);
        fillPSRAMUsageInfo(psramUsage);
        fillPowerSourceInfo(powerSource);
        fillWiFiSignalInfo(wifiSignal);
        
        sets::Group g(b, "Hardware");
        b.Label("ESP32 module", moduleSpecs);
        b.Label("ram_usage"_h, "RAM usage", ramUsage);
        if (!psramUsage.isEmpty())
            b.Label("psram_usage"_h, "PSRAM usage", psramUsage);
        b.Label("power_source"_h, "Power source", powerSource);
        b.Label("wifi_signal"_h, "WiFi signal", wifiSignal);
    }

    void Main::settingsUpdate(sets::Updater &u)
    {
        String ramUsage;
        String psramUsage;
        String powerSource;
        String wifiSignal;

        fillRAMUsageInfo(ramUsage);
        fillPSRAMUsageInfo(psramUsage);
        fillPowerSourceInfo(powerSource);
        fillWiFiSignalInfo(wifiSignal);

        u.update("ram_usage"_h, ramUsage);
        if (!psramUsage.isEmpty())
            u.update("psram_usage"_h, psramUsage);
        u.update("power_source"_h, powerSource);
        u.update("wifi_signal"_h, wifiSignal);
    }

    void Main::fillESPModuleInfo(String &moduleSpecs)
    {
        moduleSpecs = String(ESP.getChipModel()) + " / ";
        moduleSpecs += "N" + String(ESP.getFlashChipSize() / uint32_t(1024 * 1024));

        if (psramFound())
            moduleSpecs += "R" + String(esp_spiram_get_size() / uint32_t(1024 * 1024));
    }

    void Main::fillRAMUsageInfo(String &ramUsage)
    {
        auto usedBytes = ESP.getHeapSize() - ESP.getFreeHeap();
        auto usedPercents = float(usedBytes) / ESP.getHeapSize() * 100.f;
        ramUsage = String(usedBytes / 1024) + "KB / " + String(usedPercents, 1) + "%";
    }

    void Main::fillPSRAMUsageInfo(String &psramUsage)
    {
        if (psramFound())
        {
            auto usedBytes = ESP.getPsramSize() - ESP.getFreePsram();
            auto usedPercents = float(usedBytes) / ESP.getPsramSize() * 100.f;
            psramUsage = String(usedBytes / 1024) + "KB / " + String(usedPercents, 1) + "%";
        }
    }

    void Main::fillPowerSourceInfo(String &powerSource)
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

    void Main::fillWiFiSignalInfo(String &wifiSignal)
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
}
