#include "MainTab.h"
#include "drivers/Display.h"
#include "drivers/PowerSource.h"
#include "services/DateTime.h"
#include "services/GeoLocation.h"
#include "services/WiFiConnection.h"
#include "services/SettingsWebApp.h"
#include "services/AudioPlayer.h"

const char custom_html[] = 
R"raw(
<html>
<body>
<div style="display: flex; justify-content: space-between; align-items: center; width: 100%;">
<img src="https://github.com/Yevgeniy-Olexandrenko/smalltv-esp32s3-board/blob/main/support/shells/smalltv_%s.png?raw=true" style="height:100px; width:auto;">
<div style="text-align:right; padding-right: 10px;">
<div style="font-size:50px;">%s</div>
<div style="font-size:20px;">%s</div>
</div>
</div>
</body>
</html>
)raw";

namespace service::details
{
    void MainTab::settingsBuild(sets::Builder &b)
    {
        briefInfoBuild(b);
        audioPlayerBuild(b);
        hardwareInfoBuild(b);
    }

    void MainTab::settingsUpdate(sets::Updater &u)
    {
        briefInfoUpdate(u);
        audioPlayerUpdate(u);
        hardwareInfoUpdate(u);
    }

    void MainTab::briefInfoBuild(sets::Builder &b)
    {
        b.beginGuest();
        {
            b.HTML("html"_h, "", getHTML());
            b.Label("weather"_h, getLocality(), getWeather());
        }
        {
            sets::Row r(b, "", sets::DivType::Line);
            b.Label("internet"_h, "🌐 Internet", getInternet());
            b.Label("uptime"_h, "⏳ Uptime", getUptime());
        }
        {
            sets::Row r(b, "", sets::DivType::Line);
            #ifndef NO_VIDEO
            if (b.Slider(display::brightness, "🔆 Brightness", 0, 100))
            {
                auto brightness = (float(b.build.value) * 0.01f);
                driver::display.setBrightness(brightness);
            }
            #endif
            #ifndef NO_AUDIO
            service::audioPlayer.getUI().settingsBuildVolume(b);
            #endif
        }
        b.endGuest();
    }

    void MainTab::briefInfoUpdate(sets::Updater &u)
    {
        u.update("html"_h, getHTML());
        u.update("weather"_h, getWeather());
        u.update("internet"_h, getInternet());
        u.update("uptime"_h, getUptime());
    }

    void MainTab::audioPlayerBuild(sets::Builder &b)
    {
        #ifndef NO_AUDIO
        b.beginGuest();
        service::audioPlayer.getUI().settingsBuild(b);
        b.endGuest();
        #endif
    }

    void MainTab::audioPlayerUpdate(sets::Updater &u)
    {
        #ifndef NO_AUDIO
        service::audioPlayer.getUI().settingsUpdate(u);
        #endif
    }

    void MainTab::hardwareInfoBuild(sets::Builder &b)
    {
        sets::Group g(b, "📄 Hardware");
        b.beginGuest();
        b.Label("ESP32 module", getESPModuleInfo());
        b.endGuest();
        b.Label("ram_usage"_h, "RAM usage", getRAMUsageInfo());
        #ifndef NO_PSRAM
        b.Label("psram_usage"_h, "PSRAM usage", getPSRAMUsageInfo());
        #endif
        b.beginGuest();
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
        b.endGuest();
    }

    void MainTab::hardwareInfoUpdate(sets::Updater &u)
    {
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

    String MainTab::getHTML() const
    {
        String casingColor = service::settingsWebApp.getSets().getCasingColor();
        casingColor.toLowerCase();

        char buffer[sizeof(custom_html) + 32];
        sprintf(buffer, custom_html,
            casingColor.c_str(),
            service::dateTime.timeToString().c_str(),
            service::dateTime.dateToString().c_str());
        return String(buffer);
    }

    String MainTab::getLocality() const
    {
        if (service::geoLocation.hasLocality())
            return 
                service::geoLocation.getCountryFlag() + 
                " " + service::geoLocation.getLocality();
        else
            return 
                "📍 " + String(service::geoLocation.getLatitude(), 4) + 
                ", " + String(service::geoLocation.getLongitude(), 4);
    }

    String MainTab::getWeather() const
    {
        return "Clouds +26 °C 🌤️";
    }

    String MainTab::getInternet() const
    {
        if (service::wifiConnection.isInternetAvailable()) return led(Led::G);
        return led(Led::R);
    }

    String MainTab::getUptime() const
    {
        auto u = millis() / (60 * 1000);
        auto m = uint8_t(u % 60); u /= 60;
        auto h = uint8_t(u % 24); u /= 24;

        char buffer[10];
        sprintf(buffer, "%d:%02d:%02d", u, h, m);
        return String(buffer);
    }

    String MainTab::getESPModuleInfo() const
    {
        String info = String(ESP.getChipModel()) + " / ";
        info += "N" + String(ESP.getFlashChipSize() / uint32_t(1024 * 1024));

        if (psramFound())
            info += "R" + String(esp_spiram_get_size() / uint32_t(1024 * 1024));
        return info;
    }

    String MainTab::getRAMUsageInfo() const
    {
        auto usedBytes = ESP.getHeapSize() - ESP.getFreeHeap();
        auto usedPercents = float(usedBytes) / ESP.getHeapSize() * 100.f;
        return String(usedBytes / 1024) + "KB / " + String(usedPercents, 1) + "%";
    }

    String MainTab::getPSRAMUsageInfo() const
    {
        auto usedBytes = ESP.getPsramSize() - ESP.getFreePsram();
        auto usedPercents = float(usedBytes) / ESP.getPsramSize() * 100.f;
        return String(usedBytes / 1024) + "KB / " + String(usedPercents, 1) + "%";
    }

    String MainTab::getPowerSourceInfo() const
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

    String MainTab::getWiFiSignalInfo() const
    {
        auto rssi = service::wifiConnection.getRSSI();
        String info = String(rssi) + "dBm";
        switch (service::wifiConnection.getSignal(rssi))
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
