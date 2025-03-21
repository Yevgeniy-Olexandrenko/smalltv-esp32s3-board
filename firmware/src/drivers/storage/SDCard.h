#pragma once
#ifndef NO_SDCARD

#include <driver/sdmmc_types.h>
#include "FatFS.h"

namespace driver
{
    class SDCard final : public FatFS
    {
    public:
        constexpr static const char* DEFAULT_MOUNT_POINT = "/sdcard";
        enum class Interface { NONE, SPI, SDIO1, SDIO4 };
        enum class Type { NONE, SD, SDHC };

        SDCard();
        ~SDCard() override;

        bool begin(const char* mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0);
        bool begin(const char* mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3);
        bool begin(const char* mountPoint, gpio_num_t clk, gpio_num_t mosi, gpio_num_t miso, gpio_num_t cs);
        void end();

        // sd card properties
        Type getCardType() const;
        Interface getCardInterface() const;

        // partition and file system properties
        uint64_t sectorCount() const override;
        uint64_t sectorSize() const override;
        bool isMounted() const override;

        // direct access for MSC device mode
        bool mscWrBuf(uint32_t lba, uint32_t off, void* buf, uint32_t size) override;
        bool mscRdBuf(uint32_t lba, uint32_t off, void* buf, uint32_t size) override;

    private:
        int m_spiSlot;
        bool m_oneBitMode;
        sdmmc_card_t* m_card;
    };
}
#endif
