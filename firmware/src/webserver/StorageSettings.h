#pragma once

#include "SettingsProvider.h"
#include "drivers/storage/Storage.h"

namespace webserver
{
    class StorageSettingsClass : public SettingsProvider
    {
    public:
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

        driver::Storage::Type getStorageType() const;

    private:
        void fillStorageSpecs(String& specs) const;

    private:
        int _typeRollback = -1;
        bool _typeChanged = false;
    };

    extern StorageSettingsClass StorageSettings;
}
