#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/sdmmc_types.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include <sd_defines.h>
#include <FS.h>

namespace driver
{
    class SDCard : public fs::FS
    {
    public:
        static constexpr const char* DEFAULT_MOUNT_POINT = "/sdcard";

        SDCard();
        ~SDCard();

        bool begin(
            gpio_num_t clk,
            gpio_num_t cmd,
            gpio_num_t d0,
            gpio_num_t d1 = GPIO_NUM_NC,
            gpio_num_t d2 = GPIO_NUM_NC, 
            gpio_num_t d3 = GPIO_NUM_NC);

        bool begin(
            const char* mountPoint, 
            gpio_num_t clk,
            gpio_num_t cmd, 
            gpio_num_t d0,
            gpio_num_t d1 = GPIO_NUM_NC,
            gpio_num_t d2 = GPIO_NUM_NC,
            gpio_num_t d3 = GPIO_NUM_NC);

//        void begin(gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs);
//        void begin(const char *mountPoint, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs);
        
        void end();

        bool isMounted() const;
        const char* getMountPoint() const;

        sdcard_type_t getType() const;
        uint64_t getSize() const;

        size_t getSectorCount() const;
        size_t getSectorSize() const;
        
        bool writeSectors(uint8_t *src, size_t startSector, size_t sectorCount);
        bool readSectors(uint8_t *dst, size_t startSector, size_t sectorCount);

    private:
        sdmmc_card_t* _card;
        SemaphoreHandle_t _mutex;
    };

    extern SDCard sdcard;
}
