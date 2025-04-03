#include <GyverNTP.h>
#include "Main.h"
#include "drivers/Drivers.h"
#include "services/wifi_connection/WiFiConnection.h"
#include "services/settings_webapp/SettingsWebApp.h"
#include "services/audio_player/AudioPlayer.h"
#include "settings.h"

const char custom_html[] = 
R"raw(
<html>
<body>
<div style="display: flex; justify-content: space-between; align-items: center; width: 100%;">
<img src="https://github.com/Yevgeniy-Olexandrenko/smalltv-esp32s3-board/blob/main/datasheet/images/smalltv_%s.png?raw=true" style="height:100px; width:auto;">
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
        bool hasInternet = service::wifiConnection.isInternetAccessible();
        bool ntpSynced = NTP.synced();

        //if (hasInternet && ntpSynced)
        {
            b.HTML("html"_h, "", getHTML());
            b.Label("📍 Kharkov", "Clouds +26 °C 🌤");
        }

        {
            sets::Row r(b, "", sets::DivType::Line);
            b.Label("internet"_h, "🌐 Internet", getInet());
            b.Label("uptime"_h, "⏳ Uptime", getUptime());
        }
        
        {
            sets::Row r(b, "", sets::DivType::Line);
            #ifndef NO_VIDEO
            if (b.Slider(db::lcd_brightness, "🔆 Brightness", 0, 100))
            {
                auto brightness = (float(b.build.value) * 0.01f);
                driver::display.setBrightness(brightness);
            }
            #endif
            #ifndef NO_AUDIO
            service::audioPlayer.getUI().settingsBuildVolume(b);
            #endif
        }
        
        #ifndef NO_AUDIO
        service::audioPlayer.getUI().settingsBuild(b);
        #endif
        {
            sets::Group g(b, "📄 Hardware");
            b.Label("ESP32 module", getESPModuleInfo());
            b.Label("ram_usage"_h, "RAM usage", getRAMUsageInfo());
            #ifndef NO_PSRAM
            b.Label("psram_usage"_h, "PSRAM usage", getPSRAMUsageInfo());
            #endif
            #ifndef NO_VINSENSE
            b.Label("power_source"_h, "Power source", getPowerSourceInfo());
            #endif
            if (!service::wifiConnection.isInAccessPointMode())
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
        bool hasInternet = service::wifiConnection.isInternetAccessible();
        bool ntpSynced = NTP.synced();

        //if (hasInternet && ntpSynced)
        {
            u.update("html"_h, getHTML());
        }
        #ifndef NO_AUDIO
        service::audioPlayer.getUI().settingsUpdate(u);
        #endif
        u.update("internet"_h, getInet());
        u.update("uptime"_h, getUptime());
        u.update("ram_usage"_h, getRAMUsageInfo());
        #ifndef NO_PSRAM
        u.update("psram_usage"_h, getPSRAMUsageInfo());
        #endif
        #ifndef NO_VINSENSE
        u.update("power_source"_h, getPowerSourceInfo());
        #endif
        if (!service::wifiConnection.isInAccessPointMode())
        {
            u.update("wifi_signal"_h, getWiFiSignalInfo());
        }
    }

    String Main::getHTML() const
    {
        String casingColor = service::settingsWebApp.sets().getCasingColor();
        casingColor.toLowerCase();

        char buffer[sizeof(custom_html) + 32];
        sprintf(buffer, custom_html,
            casingColor.c_str(),
            NTP.timeToString().c_str(),
            NTP.dateToString().c_str());
        return String(buffer);
    }

    String Main::getInet() const
    {
        if (service::wifiConnection.isInternetAccessible()) return led(Led::G);
        return led(Led::R);
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
        #ifndef NO_VINSENSE
        auto voltage = driver::powerSource.getInputVoltage();
        switch(driver::powerSource.getType())
        {
            case driver::PowerSource::Type::Unknown:
                return String(voltage) + "V ⚡";

            case driver::PowerSource::Type::Battery:
            {   auto level = driver::powerSource.getBatteryLevelPercents();
                return "BAT " + String(voltage) + "V / " + String(level) + "% 🔋";
            }

            case driver::PowerSource::Type::USB:
                return "USB " + String(voltage) + "V 🔌";
        }
        #endif
        return "";
    }

    String Main::getWiFiSignalInfo() const
    {
        auto rssi = service::wifiConnection.getSignalRSSI();
        String info = String(rssi) + "dBm";
        switch (service::wifiConnection.getSignalQuality(rssi))
        {
            case service::WiFiConnection::Signal::Excellent:
                info += " Excellent " + led(Led::G);
                break;
            case service::WiFiConnection::Signal::Good:
                info += " Good " + led(Led::G);
                break;
            case service::WiFiConnection::Signal::Fair:
                info += " Fair " + led(Led::Y); 
                break;
            case service::WiFiConnection::Signal::Bad:
                info += " Bad " + led(Led::R); 
                break;
        }
        return info;
    }
}
