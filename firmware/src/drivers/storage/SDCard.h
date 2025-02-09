#pragma once

#include <FS.h>
#include <driver/sdmmc_types.h>
#include "shared/tasks/Mutex.h"

namespace driver
{
    class SDCard : public fs::FS
    {
    public:
        constexpr static const char* DEFAULT_MOUNT_POINT = "/sdcard";
        enum class Interface { NONE, SPI, SDIO1, SDIO4 };
        enum class Type { NONE, SD, SDHC };

        SDCard();
        ~SDCard();

        bool begin(const char* mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0);
        bool begin(const char* mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3);
        bool begin(const char* mountPoint, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs);
        void end();

        // sd card properties
        Type getCardType() const;
        Interface getCardInterface() const;

        // partition properties
        uint64_t getSectorSize() const;
        uint64_t getSectorCount() const;
        uint64_t getPartitionSize() const;

        // file system properties
        bool isMounted() const;
        const char* getMountPoint() const;
        int getDriveNumber() const;

        // direct access for MSC device mode
        bool writeSectors(uint8_t *src, size_t startSector, size_t sectorCount);
        bool readSectors(uint8_t *dst, size_t startSector, size_t sectorCount);

    private:
        int m_spiSlot;
        bool m_oneBitMode;
        sdmmc_card_t* m_card;
        task::Mutex m_mutex;
    };

    extern SDCard sdcard;
}
