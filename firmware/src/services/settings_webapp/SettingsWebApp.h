#pragma once

#include "shared/settings/Settings.h"
#include "tabs/Apps.h"
#include "tabs/Main.h"
#include "tabs/Sets.h"

namespace service
{
    using namespace service_settings_webapp_impl;

    class SettingsWebApp
    {
    public:
        void begin();
        void update();
        
        void requestDeviceReboot();

        const Sets& sets() const { return m_tabSets; }
        const Main& main() const { return m_tabMain; }
        const Apps& apps() const { return m_tabApps; }

    private:
        void settingsBuild(sets::Builder& b);
        void settingsUpdate(sets::Updater& u);
        void onFocusChange(bool f);

    private:
        uint8_t m_currentTab;
        bool m_connectedToPC;
        bool m_requestReboot;

        Sets m_tabSets;
        Main m_tabMain;
        Apps m_tabApps;
    };

    extern SettingsWebApp settingsWebApp;
}