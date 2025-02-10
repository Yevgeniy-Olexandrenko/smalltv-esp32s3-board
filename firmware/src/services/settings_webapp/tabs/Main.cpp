#include <GyverNTP.h>
#include "Main.h"
#include "drivers/Drivers.h"
#include "services/network_connection/NetworkConnection.h"
#include "services/settings_webapp/SettingsWebApp.h"
#include "settings.h"

const char custom_html[] = 
R"raw(
<html>
<body>
<div style="display: flex; justify-content: space-between; align-items: center; width: 100%;">
<img src="https://www.espressif.com/sites/default/files/chips/ESP32-S3_L.png" style="height:100px; width:auto;">
<div style="text-align:right; padding-right: 10px;">
<div style="font-size:50px;">%s</div>
<div style="font-size:20px;">%s</div>
</div>
</div>
</body>
</html>
)raw";

namespace service_settings_webapp_impl
{
    void Main::settingsBuild(sets::Builder &b)
    {
        bool hasInternet = service::networkConnection.isInternetAccessible();
        bool ntpSynced = NTP.synced();

        //if (hasInternet && ntpSynced)
        {
            b.HTML("html"_h, "", getHTML());
        }

        {
            sets::Row r(b, "", sets::DivType::Block);
            b.LED("internet"_h, "Internet", hasInternet);
            b.Label("uptime"_h, "Uptime", getUptime());
        }

        {
            sets::Row r(b, "Controls", sets::DivType::Block);
            if (b.Slider(db::lcd_brightness, "Brightness", 0, 200))
            {
                auto brightness = driver::LCDBacklight::Brightness(b.build.value);
                driver::lcdBacklight.setBrightness(brightness);
            }
            if (hardware::hasAudio())
            {
                if (b.Slider(db::audio_volume, "Volume", 0, 200))
                {
                    // TODO
                }
            }
        }

        {
            sets::Group g(b, "Hardware");
            b.Label("ESP32 module", getESPModuleInfo());
            b.Label("ram_usage"_h, "RAM usage", getRAMUsageInfo());
            if (hardware::hasPSRAM())
            {
                b.Label("psram_usage"_h, "PSRAM usage", getPSRAMUsageInfo());
            }
            if (hardware::hasVINSense())
            {
                b.Label("power_source"_h, "Power source", getPowerSourceInfo());
            }
            if (!service::networkConnection.isInAccessPointMode())
            {
                b.Label("wifi_signal"_h, "WiFi signal", getWiFiSignalInfo());
            }
            if (b.Button("Reboot"))
            {
                service::settingsWebApp.requestReboot(nullptr);
            }
        }
    }

    void Main::settingsUpdate(sets::Updater &u)
    {
        bool hasInternet = service::networkConnection.isInternetAccessible();
        bool ntpSynced = NTP.synced();

        //if (hasInternet && ntpSynced)
        {
            u.update("html"_h, getHTML());
        }
        u.update("internet"_h, hasInternet);
        u.update("uptime"_h, getUptime());
        u.update("ram_usage"_h, getRAMUsageInfo());
        if (hardware::hasPSRAM())
        {
            u.update("psram_usage"_h, getPSRAMUsageInfo());
        }
        if (hardware::hasVINSense())
        {
            u.update("power_source"_h, getPowerSourceInfo());
        }
        if (!service::networkConnection.isInAccessPointMode())
        {
            u.update("wifi_signal"_h, getWiFiSignalInfo());
        }
    }

    String Main::getHTML() const
    {
        char buffer[sizeof(custom_html) + 20];
        sprintf(buffer, custom_html, NTP.timeToString().c_str(), NTP.dateToString().c_str());
        return String(buffer);
    }

    String Main::getUptime() const
    {
        auto u = millis() / (60 * 1000);
        auto m = uint8_t(u % 60); u /= 60;
        auto h = uint8_t(u % 24); u /= 24;
        return String(u) + "d " + String(h) + "h " + String(m) + "m";
    }

    String Main::getESPModuleInfo() const
    {
        String info = String(ESP.getChipModel()) + " / ";
        info += "N" + String(ESP.getFlashChipSize() / uint32_t(1024 * 1024));

        if (psramFound())
            info += "R" + String(esp_spiram_get_size() / uint32_t(1024 * 1024));
        return info;
    }

    String Main::getRAMUsageInfo() const
    {
        auto usedBytes = ESP.getHeapSize() - ESP.getFreeHeap();
        auto usedPercents = float(usedBytes) / ESP.getHeapSize() * 100.f;
        return String(usedBytes / 1024) + "KB / " + String(usedPercents, 1) + "%";
    }

    String Main::getPSRAMUsageInfo() const
    {
        auto usedBytes = ESP.getPsramSize() - ESP.getFreePsram();
        auto usedPercents = float(usedBytes) / ESP.getPsramSize() * 100.f;
        return String(usedBytes / 1024) + "KB / " + String(usedPercents, 1) + "%";
    }

    String Main::getPowerSourceInfo() const
    {
        auto voltage = driver::powerSource.getInputVoltage();
        switch(driver::powerSource.getType())
        {
            case driver::PowerSource::Type::Unknown:
                return String(voltage) + "V";

            case driver::PowerSource::Type::Battery:
            {   auto level = driver::powerSource.getBatteryLevelPercents();
                return "BAT " + String(voltage) + "V / " + String(level) + "%";
            }

            case driver::PowerSource::Type::USB:
                return "USB " + String(voltage) + "V";
        }
        return "";
    }

    String Main::getWiFiSignalInfo() const
    {
        String info = String(service::networkConnection.getSignalRSSI()) + "dBm";
        switch (service::networkConnection.getSignalStrength())
        {
            case service::NetworkConnection::Signal::Excellent: info += " Excellent"; break;
            case service::NetworkConnection::Signal::Good: info += " Good"; break;
            case service::NetworkConnection::Signal::Fair: info += " Fair"; break;
            case service::NetworkConnection::Signal::Bad: info += " Bad"; break;
        }
        return info;
    }
}
