#pragma once
#ifndef NO_SDCARD

#include <driver/sdmmc_types.h>
#include "FatFS.h"

namespace driver::details
{
    class SDCard final : public FatFS
    {
    public:
        constexpr static const char* DEFAULT_MOUNT_POINT = "/sdcard";
        enum class Interface { NONE, SPI, SDIO1, SDIO4 };
        enum class Type { NONE, SD, SDHC };

        SDCard();
        ~SDCard() override;

        bool begin(const char* mountPoint, int clk, int cmd, int d0);
        bool begin(const char* mountPoint, int clk, int cmd, int d0, int d1, int d2, int d3);
        bool begin(const char* mountPoint, int sck, int mosi, int miso, int cs);
        void end();

        // sd card properties
        Type getCardType() const;
        Interface getCardInterface() const;

        // partition and file system properties
        uint64_t sectorCount() const override;
        uint64_t sectorSize() const override;
        bool isMounted() const override;

        // direct access for MSC device mode
        bool writeSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount) override;
        bool readSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount) override;

    private:
        int m_spiSlot;
        bool m_oneBitMode;
        sdmmc_card_t* m_card;
    };
}
#endif
