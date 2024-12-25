#pragma once

#include <FS.h>
#include <wear_levelling.h>
#include <freertos/FreeRTOS.h>

namespace driver
{
    class Flash : public fs::FS
    {
    public:
        static constexpr const char* DEFAULT_MOUNT_POINT = "/flash";

        Flash();
        ~Flash();

        bool begin(const char* mountPoint, const char* partitionLabel = "ffat");
        void end();

        bool isMounted() const;
        const char* getMountPoint() const;
        size_t getPartitionSize() const;
        size_t getSectorCount() const;
        size_t getSectorSize() const;

        size_t getTotalBytes();
        size_t getUsedBytes();

        bool writeSectors(uint8_t *src, size_t startSector, size_t sectorCount);
        bool readSectors(uint8_t *dst, size_t startSector, size_t sectorCount);

    private:
        wl_handle_t _wl_handle;
        SemaphoreHandle_t _mutex;
    };

    extern Flash flash;
}
