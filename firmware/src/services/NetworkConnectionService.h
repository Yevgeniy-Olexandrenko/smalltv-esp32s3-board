#pragma once

#include <GyverDBFile.h>
#include <SettingsBase.h>
#include "BackgroundService.h"

class NetworkConnectionServiceClass : public BackgroundService
{
    enum class State { ConnectRequested, Connecting, Connected, NotConnected, ScanRequested, Scanning };

public:
    void begin();
    void update();

    void settingsBuild(sets::Builder& b);
    void settingsUpdate(sets::Updater& u);

private:
    void buildWiFiScanResult(sets::Builder& b, int max);
    void buildWiFiConnection(sets::Builder& b);

private:
    State m_state = State::NotConnected;
    String m_ssid;
    String m_pass;
};

extern NetworkConnectionServiceClass NetworkConnectionService;
