#pragma once

#include "shared/tasks/Task.h"
#include "shared/settings/Settings.h"
#include "tabs/Apps.h"
#include "tabs/Main.h"
#include "tabs/Sets.h"

namespace service
{
    using namespace service_settings_webapp_impl;
    using RebootCorfirmCB = std::function<void(bool)>;

    class SettingsWebApp 
        : public task::Task<8192 * 2, task::core::Application, task::priority::Background>
    {
    public:
        void begin();
        void requestReboot(RebootCorfirmCB cb);

        const Sets& sets() const { return m_tabSets; }
        const Main& main() const { return m_tabMain; }
        const Apps& apps() const { return m_tabApps; }

    private:
        void task() override;
        void settingsBuild(sets::Builder& b);
        void settingsUpdate(sets::Updater& u);
        void onFocusChange(bool f);

    private:
        uint8_t m_currentTab;
        bool m_connectedToPC;
        
        bool m_rebootRequest;
        bool m_rebootPending;
        RebootCorfirmCB m_rebootConfirmCB;

        Sets m_tabSets;
        Main m_tabMain;
        Apps m_tabApps;
    };

    extern SettingsWebApp settingsWebApp;
}