// #pragma once

// #include "core/tasks/Task.h"
// #include "core/settings/Settings.h"
// #include "SettingsWebApp/AppsTab.h"
// #include "SettingsWebApp/MainTab.h"
// #include "SettingsWebApp/SetsTab.h"

// namespace service
// {
//     using RebootCorfirmCB = std::function<void(bool)>;

//     class SettingsWebApp 
//         : public task::Task<8192, task::core::Application, task::priority::Background>
//     {
//     public:
//         void begin();
//         void requestReboot(RebootCorfirmCB cb);

//         details::SetsTab& getSets() { return m_sets; };
//         details::MainTab& getMain() { return m_main; };
//         details::AppsTab& getApps() { return m_apps; };

//     private:
//         void task() override;
//         void settingsBuild(sets::Builder& b);
//         void settingsUpdate(sets::Updater& u);
//         void onFocusChange(bool f);

//     private:
//         uint8_t m_currentTab;
//         bool m_connectedToPC;
        
//         bool m_rebootRequest;
//         bool m_rebootPending;
//         RebootCorfirmCB m_rebootConfirmCB;

//         details::SetsTab m_sets;
//         details::MainTab m_main;
//         details::AppsTab m_apps;
//     };

//     extern SettingsWebApp settingsWebApp;
// }