#pragma once
#ifndef NO_PSRAM

#include "fatfs/psram_partition.h"
#include "FatFS.h"

namespace driver::details
{
    class PSRam final : public FatFS
    {
    public:
        constexpr static auto DEFAULT_MOUNT_POINT = "/psram";

        PSRam();
        ~PSRam() override;

        bool begin(const char* mountPoint, float portion = 0.5f);
        bool begin(const char* mountPoint, uint32_t sectors);
        void end();

        // partition and file system properties
        uint64_t sectorCount() const override;
        uint64_t sectorSize() const override;
        bool isMounted() const override;

        // direct access for MSC device mode
        bool writeSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount) override;
        bool readSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount) override;

    private:
        psram_partition_t* m_psram;
    };
}
#endif
