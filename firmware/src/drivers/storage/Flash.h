#pragma once
#ifndef NO_FLASH

#include <wear_levelling.h>
#include "FatFS.h"

namespace driver
{
    class Flash final : public FatFS
    {
    public:
        constexpr static const char* DEFAULT_MOUNT_POINT = "/flash";
        constexpr static const char* DEFAULT_PARTITION_LABEL = "ffat";

        Flash();
        ~Flash() override;

        bool begin(const char* mountPoint, const char* partitionLabel = DEFAULT_PARTITION_LABEL);
        void end();

        // partition and file system properties
        uint64_t sectorCount() const override;
        uint64_t sectorSize() const override;
        bool isMounted() const override;

        // direct access for MSC device mode
        bool mscWrBuf(uint32_t lba, uint32_t off, void* buf, uint32_t size) override;
        bool mscRdBuf(uint32_t lba, uint32_t off, void* buf, uint32_t size) override;

    private:
        wl_handle_t m_wlHandle;
    };
}
#endif
