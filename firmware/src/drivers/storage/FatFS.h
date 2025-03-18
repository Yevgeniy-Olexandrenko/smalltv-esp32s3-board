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
        virtual uint64_t sectorCount() const;
        virtual uint64_t sectorSize() const;
        uint64_t partitionSize() const;

        // file system properties
        virtual bool isMounted() const;
        const char* mountPoint() const;
        uint64_t totalBytes() const;
        uint64_t usedBytes() const;

        // direct access for MSC device mode
        virtual bool mscWrBuf(uint32_t lba, uint32_t offset, void* buffer, uint32_t size);
        virtual bool mscRdBuf(uint32_t lba, uint32_t offset, void* buffer, uint32_t size);

    protected:
        void setMountPoint(const char* mountPoint);
        void resMountPoint();
    };
}
