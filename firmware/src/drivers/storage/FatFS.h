#pragma once

#include <FS.h>

namespace driver
{
    class FatFS : public fs::FS
    {
    public:
        FatFS();
        virtual ~FatFS() = default;

        // partition properties
        virtual uint64_t sectorCount() const = 0;
        virtual uint64_t sectorSize() const = 0;
        uint64_t partitionSize() const;

        // file system properties
        virtual bool isMounted() const = 0;
        const char* mountPoint() const;
        uint64_t totalBytes() const;
        uint64_t usedBytes() const;

    protected:
        void setMountPoint(const char* mountPoint);
        void resMountPoint();
    };
}
