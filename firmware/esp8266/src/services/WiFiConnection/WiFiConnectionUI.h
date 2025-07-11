// #pragma once

// #include "core/settings/Settings.h"

// namespace service::details
// {
//     class WiFiConnectionUI : public settings::Provider
//     {
//         enum class Action { None, DoScan, DoConnect, GoToManual };
//         struct Station { String ssid; bool open; int signal; };

//     public:
//         void begin();
//         void settingsBuild(sets::Builder& b) override;
//         void settingsUpdate(sets::Updater& u) override;

//     private:
//         void scanForStations();
//         bool isPassClosedStation() const;
//         void fillOptionsWithStations(String& options);
//         void chooseStationByIndex(size_t index);

//     private:
//         String m_ssid, m_pass;
//         Action m_action{ Action::None };
//         std::vector<Station> m_stations;
//     };
// }
