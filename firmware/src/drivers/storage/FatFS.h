#pragma once

#include <FS.h>

namespace driver::details
{
    class FatFS : public fs::FS
    {
    public:
        FatFS();
        virtual ~FatFS() = default;

        // partition properties
        virtual uint64_t sectorCount() const { return 0; }
        virtual uint64_t sectorSize() const { return 0; }
        uint64_t partitionSize() const;

        // file system properties
        virtual bool isMounted() const { return false; }
        const char* mountPoint() const;
        uint64_t totalBytes() const;
        uint64_t usedBytes() const;

        // direct access for MSC device mode
        virtual bool writeSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount) { return false; }
        virtual bool readSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount) { return false; }

    protected:
        void setMountPoint(const char* mountPoint);
        void resMountPoint();
    };
}
