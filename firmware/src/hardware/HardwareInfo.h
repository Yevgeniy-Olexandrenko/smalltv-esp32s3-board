#pragma once

#include "services/Settings/SettingsProvider.h"

class HardwareInfoClass : public SettingsProvider
{
public:
    void settingsBuild(sets::Builder& b) override;
    void settingsUpdate(sets::Updater& u) override;

private:
    String m_chip;
    String m_memory;
};

extern HardwareInfoClass HardwareInfo;
