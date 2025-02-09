#pragma once

#include <FS.h>
#include <wear_levelling.h>
#include "shared/tasks/Mutex.h"

namespace driver
{
    class Flash : public fs::FS
    {
    public:
        constexpr static const char* DEFAULT_MOUNT_POINT = "/flash";
        constexpr static const char* DEFAULT_PARTITION_LABEL = "ffat";

        Flash();
        ~Flash();

        bool begin(const char* mountPoint, const char* partitionLabel = DEFAULT_PARTITION_LABEL);
        void end();

        // partition properties
        size_t getSectorSize() const;
        size_t getSectorCount() const;
        size_t getPartitionSize() const;

        // file system properties
        bool isMounted() const;
        const char* getMountPoint() const;
        int getDriveNumber() const;

        // direct access for MSC device mode
        bool writeBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
        bool readBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);

    private:
        wl_handle_t m_wlHandle;
        task::Mutex m_mutex;
    };

    extern Flash flash;
}
