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
        static constexpr const char* DEFAULT_PARTITION_LABEL = "ffat";

        Flash();
        ~Flash();

        bool begin(const char* mountPoint, const char* partitionLabel = DEFAULT_PARTITION_LABEL);
        void end();

        bool isMounted() const;
        const char* getMountPoint() const;
        size_t getPartitionSize() const;
        size_t getSectorCount() const;
        size_t getSectorSize() const;

        size_t getTotalBytes() const;
        size_t getUsedBytes() const;

        bool writeBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
        bool readBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);

    private:
        wl_handle_t _wl_handle;
        SemaphoreHandle_t _mutex;
    };

    extern Flash flash;
}
