#pragma once

#include <SettingsBase.h>

class SettingsProvider
{
public:
    virtual void settingsBuild(sets::Builder& b) = 0;
    virtual void settingsUpdate(sets::Updater& u) = 0;
};
