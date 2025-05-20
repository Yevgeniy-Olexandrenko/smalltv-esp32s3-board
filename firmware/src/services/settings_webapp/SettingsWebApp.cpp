#include <GyverNTP.h>
#include "SettingsWebApp.h"
#include "drivers/onboard/SelfReboot.h"

namespace service
{
    void SettingsWebApp::begin()
    {
        log_i("begin");
        Task::start("settings_webapp");
    }

    void SettingsWebApp::requestReboot(RebootCorfirmCB cb)
    {
        m_rebootConfirmCB = cb;
        m_rebootRequest = true;
    }

    void SettingsWebApp::task()
    {
        m_sets.begin();
        m_main.begin();
        m_apps.begin();

        settings::sets().onBuild([this](sets::Builder& b) { this->settingsBuild(b); });
        settings::sets().onUpdate([this](sets::Updater& u) { this->settingsUpdate(u); });
        settings::sets().onFocusChange([this]() { this->onFocusChange(settings::sets().focused()); });
        settings::sets().rtc.onSync([](uint32_t unix) { NTP.sync(unix); });

        settings::sets().config.updateTout = 1000;
        settings::sets().config.theme = m_sets.getThemeColor();
        settings::sets().setProjectInfo("home page", WEBAPP_PROJECT_HOME);
        settings::sets().setPass("0000");

        m_currentTab = 1;
        m_connectedToPC = false;
        m_rebootRequest = false;
        m_rebootPending = false;

        while(true)
        {
            settings::sets().tick();
            if (m_rebootPending)
            {
                m_currentTab = 1;
                m_rebootPending = false;
                driver::selfReboot.reboot();
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    void SettingsWebApp::settingsBuild(sets::Builder &b)
    {
        if (m_connectedToPC)
        {
            b.beginGuest();
            b.Paragraph("Connected to PC", 
                "This device is connected to your computer as an external "
                "storage device. Please use \"Safely Remove\" or \"Eject\" "
                "operation to stop working with it!");
            b.endGuest();
            return;
        }

        b.beginGuest();
        if (b.Tabs("🛠 SETS;💜 MAIN;🚀 APPS", &m_currentTab))
        {
            b.reload();
            return;
        }
        b.endGuest();

        switch(m_currentTab)
        {
        case 0: m_sets.settingsBuild(b); break;
        case 1: m_main.settingsBuild(b); break;
        case 2: m_apps.settingsBuild(b); break;
        }

        b.beginGuest();
        if (b.Confirm("reboot_confirm"_h, "This operation requires a device reboot!"))
        {
            if (m_rebootConfirmCB)
                m_rebootConfirmCB(b.build.value.toBool());

            if (b.build.value.toBool())
            {
                m_rebootPending = true;
                settings::data().update();
                settings::sets().reload();
            }
        }
        b.endGuest();
    }

    void SettingsWebApp::settingsUpdate(sets::Updater &u)
    {
        if (m_connectedToPC != driver::storage.isMSCRunning())
        {
            m_connectedToPC ^= true;
            settings::sets().reload();
            return;
        }

        switch(m_currentTab)
        {
        case 0: m_sets.settingsUpdate(u); break;
        case 1: m_main.settingsUpdate(u); break;
        case 2: m_apps.settingsUpdate(u); break;
        }

        if (m_rebootRequest)
        {
            m_rebootRequest = false;
            u.update("reboot_confirm"_h);
        }
    }

    void SettingsWebApp::onFocusChange(bool f)
    {
        log_i("focus: %s", (f ? "YES" : "NO"));
    }

    SettingsWebApp settingsWebApp;
}
