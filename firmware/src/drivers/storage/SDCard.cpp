#include "SDCard.h"
#include <vfs_api.h>
#include <esp_vfs_fat.h>
#include <driver/sdmmc_defs.h>
#include <sdmmc_cmd.h>
#include <esp32-hal-log.h>

namespace driver
{
    SDCard::SDCard()
        : FS(FSImplPtr(new VFSImpl()))
        , m_card(nullptr)
    {
    }

    SDCard::~SDCard()
    {
        end();
    }

    // void SDCard::begin(gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs)
    // {
    //     begin(DEFAULT_MOUNT_POINT, miso, mosi, clk, cs);
    // }

    void SDCard::begin(gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3)
    {
        begin(DEFAULT_MOUNT_POINT, clk, cmd, d0, d1, d2, d3);
    }

//     void SDCard::begin(const char *mountPoint, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs)
//     {
//         log_i("Initializing SD card");
//
//         m_host = SDSPI_HOST_DEFAULT();
//         m_host.max_freq_khz = SDMMC_FREQ_52M;
// #ifdef SD_CARD_SPI_HOST
//         // only enable on ESP32
//         m_host.slot = SD_CARD_SPI_HOST;
// #endif
//         spi_bus_config_t bus_cfg = 
//         {
//             .mosi_io_num = mosi,
//             .miso_io_num = miso,
//             .sclk_io_num = clk,
//             .quadwp_io_num = GPIO_NUM_NC,
//             .quadhd_io_num = GPIO_NUM_NC,
//             .max_transfer_sz = 16384,
//             .flags = 0,
//             .intr_flags = 0
//         };
//
//         esp_err_t ret = spi_bus_initialize(spi_host_device_t(m_host.slot), &bus_cfg, SPI_DMA_CH_AUTO);
//         if (ret != ESP_OK)
//         {
//             log_e("Initializing SD card failed: %s\n", esp_err_to_name(ret));
//             return;
//         }
//
//         esp_vfs_fat_sdmmc_mount_config_t mount_config = 
//         {
//             .format_if_mount_failed = false,
//             .max_files = 5,
//             .allocation_unit_size = 16384
//         };
//
//         // This initializes the slot without card detect (CD) and write protect (WP) signals.
//         // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
//         sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
//         slot_config.gpio_cs = cs;
//         slot_config.host_id = spi_host_device_t(m_host.slot);
//
//         ret = esp_vfs_fat_sdspi_mount(mountPoint, &m_host, &slot_config, &mount_config, &m_card);
//         if (ret != ESP_OK)
//         {
//             log_e("Initializing SD card failed: %s\n", esp_err_to_name(ret));
//             return;
//         }
//
//         log_i("SD card mounted at: %s\n", mountPoint);
//         sdmmc_card_print_info(stdout, m_card);
//
//         _impl->mountpoint(mountPoint);
//         m_mutex = xSemaphoreCreateMutex();
//     }

    void SDCard::begin(const char *mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3)
    {
        log_i("Initializing SD card");

        sdmmc_host_t m_host = SDMMC_HOST_DEFAULT();
        m_host.max_freq_khz = SDMMC_FREQ_52M;
        m_host.flags = SDMMC_HOST_FLAG_4BIT;

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        slot_config.flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
        slot_config.width = 4;
        slot_config.clk = clk;
        slot_config.cmd = cmd;
        slot_config.d0 = d0;
        slot_config.d1 = d1;
        slot_config.d2 = d2;
        slot_config.d3 = d3;
        slot_config.d4 = GPIO_NUM_NC;
        slot_config.d5 = GPIO_NUM_NC;
        slot_config.d6 = GPIO_NUM_NC;
        slot_config.d7 = GPIO_NUM_NC;

        esp_vfs_fat_sdmmc_mount_config_t mount_config = 
        {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 16384
        };

        esp_err_t ret = esp_vfs_fat_sdmmc_mount(mountPoint, &m_host, &slot_config, &mount_config, &m_card);
        if (ret != ESP_OK)
        {
            log_e("Initializing SD card failed: %s\n", esp_err_to_name(ret));
            return;
        }

        log_i("SD card mounted at: %s\n", mountPoint);
        sdmmc_card_print_info(stdout, m_card);
        
        _impl->mountpoint(mountPoint);
        m_mutex = xSemaphoreCreateMutex();
    }

    void SDCard::end()
    {
        if (m_card) 
        {
            esp_vfs_fat_sdcard_unmount(_impl->mountpoint(), m_card);
            //spi_bus_free(spi_host_device_t(m_host.slot)); // TODO
            _impl->mountpoint(nullptr);
            m_card = nullptr;
        }
    }

    bool SDCard::isMounted() const
    {
        return (getMountPoint() != nullptr);
    }

    const char *SDCard::getMountPoint() const
    {
        return _impl->mountpoint();
    }

    sdcard_type_t SDCard::getType() const
    {
        if(!m_card) return CARD_NONE;
        return (m_card->ocr & SD_OCR_SDHC_CAP ? CARD_SDHC : CARD_SD);
    }

    uint64_t SDCard::getSize() const
    {
        uint64_t sectorSize = getSectorSize();
        uint64_t sectorCount = getSectorCount();
        return (sectorSize * sectorSize);
    }

    size_t SDCard::getSectorCount() const
    {
        return (m_card ? m_card->csd.capacity : 0);
    }

    size_t SDCard::getSectorSize() const
    {
        return (m_card ? m_card->csd.sector_size : 0);
    }

    bool SDCard::writeSectors(uint8_t *src, size_t startSector, size_t sectorCount)
    {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        //digitalWrite(GPIO_NUM_2, HIGH);
        esp_err_t res = sdmmc_write_sectors(m_card, src, startSector, sectorCount);
        //digitalWrite(GPIO_NUM_2, LOW);
        xSemaphoreGive(m_mutex);
        return (res == ESP_OK);
    }

    bool SDCard::readSectors(uint8_t *dst, size_t startSector, size_t sectorCount)
    {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        //digitalWrite(GPIO_NUM_2, HIGH);
        esp_err_t res = sdmmc_read_sectors(m_card, dst, startSector, sectorCount);
        //digitalWrite(GPIO_NUM_2, LOW);
        xSemaphoreGive(m_mutex);
        return (res == ESP_OK);
    }

    SDCard sdcard;
}
