#pragma once

#include "SettingsProvider.h"

namespace webserver
{
    class HardwareInfoClass : public SettingsProvider
    {
    public:
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        void fillESPModuleInfo(String& moduleChip, String& moduleMemory);
        void fillHeapUsageInfo(String& heapUsage);
        void fillPowerSourceInfo(String& powerSource);

    private:
        String m_moduleChip;
        String m_moduleMemory;
    };

    extern HardwareInfoClass HardwareInfo;
}
