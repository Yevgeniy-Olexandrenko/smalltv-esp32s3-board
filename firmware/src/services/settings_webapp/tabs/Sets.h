#pragma once

#include "shared/settings/Settings.h"
#include "drivers/storage/Storage.h"

namespace service_settings_webapp_impl
{
    class Sets : public settings::Provider
    {
    public:
        void begin();
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

        String getCasingColor() const;
        sets::Colors getThemeColor() const;

    private:
        void colorsSettingsBuild(sets::Builder &b);
        void storageSettingsBuild(sets::Builder &b);
        void storageSettingsUpdate(sets::Updater &u);
        void fillStorageSpecs(String &specs) const;

    private:
        int m_typeRollback = -1;
        bool m_typeChanged = false;
    };
}
