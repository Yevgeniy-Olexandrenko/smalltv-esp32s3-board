#include "Drivers.h"
#include "shared/settings/Settings.h"

namespace drivers
{
    DB_KEYS(storage, type);

    void begin()
    {
        settings::data().init(storage::type, int(driver::Storage::Type::Auto));
        auto storageType = driver::Storage::Type(int(settings::data()[storage::type]));

        driver::powerSource.begin();
        driver::storage.begin(storageType);
    }
}