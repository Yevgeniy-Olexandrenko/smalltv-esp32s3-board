#pragma once

#include "services/Service.h"
#include "services/Settings/SettingsProvider.h"

class NetworkConnectionServiceClass : public Service, public SettingsProvider
{
    enum class State { ConnectRequested, Connecting, Connected, NotConnected, ScanRequested, Scanning };

public:
    void begin();
    void update();

    void settingsBuild(sets::Builder& b) override;
    void settingsUpdate(sets::Updater& u) override;

private:
    void buildWiFiScanResult(sets::Builder& b, int max);
    void buildWiFiConnection(sets::Builder& b);

private:
    State m_state = State::NotConnected;
    String m_ssid;
    String m_pass;
};

extern NetworkConnectionServiceClass NetworkConnectionService;
