#pragma once

#include <FS.h>
#include <sd_defines.h>
#include <freertos/FreeRTOS.h>
#include <driver/sdmmc_types.h>

namespace driver
{
    class SDCard : public fs::FS
    {
    public:
        static constexpr const char* DEFAULT_MOUNT_POINT = "/sdcard";
        enum class Interface { NONE, SPI, SDIO1, SDIO4 };

        SDCard();
        ~SDCard();

        bool begin(const char* mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0);
        bool begin(const char* mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3);
        bool begin(const char* mountPoint, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs);
        void end();

        // sd card properties
        sdcard_type_t getCardType() const;
        Interface getCardInterface() const;

        // partition properties
        uint64_t getSectorSize() const;
        uint64_t getSectorCount() const;
        uint64_t getPartitionSize() const;

        // file system properties
        bool isMounted() const;
        const char* getMountPoint() const;
        uint64_t getTotalBytes() const;
        uint64_t getUsedBytes() const;

        // direct access for MSC device mode
        bool writeSectors(uint8_t *src, size_t startSector, size_t sectorCount);
        bool readSectors(uint8_t *dst, size_t startSector, size_t sectorCount);

    private:
        int _spi_slot;
        bool _onebit_mode;
        sdmmc_card_t* _card;
        SemaphoreHandle_t _mutex;
    };

    extern SDCard sdcard;
}
