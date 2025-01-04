#include "SDCard.h"
#include <vfs_api.h>
#include <esp_vfs_fat.h>
#include <driver/sdmmc_defs.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include <sdmmc_cmd.h>

namespace driver
{
    SDCard::SDCard()
        : FS(FSImplPtr(new VFSImpl()))
        , _spi_slot(-1)
        , _onebit_mode(false)
        , _card(nullptr)
    {}

    SDCard::~SDCard() { end(); }

    ////////////////////////////////////////////////////////////////////////////

    // SDIO 1 bit mode
    bool SDCard::begin(const char *mountPoint, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0)
    {
        return begin(mountPoint, clk, cmd, d0, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC);
    }

    // SDIO 4 bit mode
    bool SDCard::begin(
        const char *mountPoint,
        gpio_num_t clk,
        gpio_num_t cmd,
        gpio_num_t d0,
        gpio_num_t d1,
        gpio_num_t d2,
        gpio_num_t d3)
    {
        if (isMounted()) return true;
        log_i("Initializing SD card");
        _onebit_mode = (d1 == GPIO_NUM_NC || d2 == GPIO_NUM_NC || d3 == GPIO_NUM_NC);

        sdmmc_host_t m_host = SDMMC_HOST_DEFAULT();
        m_host.max_freq_khz = SDMMC_FREQ_52M;
        m_host.flags = (_onebit_mode ? SDMMC_HOST_FLAG_1BIT : SDMMC_HOST_FLAG_4BIT);

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        #ifdef SOC_SDMMC_USE_GPIO_MATRIX
        slot_config.width = (_onebit_mode ? 1 : 4);
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
        #endif

        esp_vfs_fat_mount_config_t mount_config = 
        {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 0
        };

        esp_err_t ret = esp_vfs_fat_sdmmc_mount(mountPoint, &m_host, &slot_config, &mount_config, &_card);
        if (ret != ESP_OK)
        {
            log_e("Initializing SD card failed: %s\n", esp_err_to_name(ret));
            _card = nullptr;
            return false;
        }

        log_i("SD card mounted at: %s\n", mountPoint);
        sdmmc_card_print_info(stdout, _card);
        
        _mutex = xSemaphoreCreateMutex();
        _impl->mountpoint(mountPoint);
        return true;
    }

    // SPI mode
    bool SDCard::begin(
        const char *mountPoint,
        gpio_num_t miso,
        gpio_num_t mosi,
        gpio_num_t clk,
        gpio_num_t cs)
    {
        if (isMounted()) return true;
        log_i("Initializing SD card");

        sdmmc_host_t m_host = SDSPI_HOST_DEFAULT();
        _spi_slot = m_host.slot;

        spi_bus_config_t bus_cfg = 
        {
            .mosi_io_num = mosi,
            .miso_io_num = miso,
            .sclk_io_num = clk,
            .quadwp_io_num = GPIO_NUM_NC,
            .quadhd_io_num = GPIO_NUM_NC,
            .max_transfer_sz = 0,
            .flags = 0,
            .intr_flags = 0
        };

        esp_err_t ret = spi_bus_initialize(spi_host_device_t(_spi_slot), &bus_cfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK) 
        {
            log_e("Initializing SPI bus failed: %s\n", esp_err_to_name(ret));
            return false;
        }

        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.host_id = spi_host_device_t(_spi_slot);
        slot_config.gpio_cs = cs;

        esp_vfs_fat_mount_config_t mount_config = 
        {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 0
        };

        ret = esp_vfs_fat_sdspi_mount(mountPoint, &m_host, &slot_config, &mount_config, &_card);
        if (ret != ESP_OK)
        {
            log_e("Initializing SD card failed: %s\n", esp_err_to_name(ret));
            _card = nullptr;
            return false;
        }

        log_i("SD card mounted at: %s\n", mountPoint);
        sdmmc_card_print_info(stdout, _card);
        
        _mutex = xSemaphoreCreateMutex();
        _impl->mountpoint(mountPoint);
        return true;
    }

    void SDCard::end()
    {
        if (isMounted()) 
        {
            xSemaphoreTake(_mutex, portMAX_DELAY);
            esp_vfs_fat_sdcard_unmount(_impl->mountpoint(), _card);
            if (_spi_slot >= 0) spi_bus_free(spi_host_device_t(_spi_slot));
            _impl->mountpoint(nullptr);
            _card = nullptr;
            xSemaphoreGive(_mutex);
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    sdcard_type_t SDCard::getCardType() const
    {
        if(!_card) return CARD_NONE;
        return (_card->ocr & SD_OCR_SDHC_CAP ? CARD_SDHC : CARD_SD);
    }

    SDCard::Interface SDCard::getCardInterface() const
    {
        if(!_card) return Interface::NONE;
        return (_spi_slot >= 0 ? Interface::SPI : (_onebit_mode ? Interface::SDIO1 : Interface::SDIO4));
    }

    ////////////////////////////////////////////////////////////////////////////

    uint64_t SDCard::getSectorSize() const
    {
        return (_card ? _card->csd.sector_size : 0);
    }

    uint64_t SDCard::getSectorCount() const
    {
        return (_card ? _card->csd.capacity : 0);
    }

    uint64_t SDCard::getPartitionSize() const
    {
        return (getSectorSize() * getSectorCount());
    }

    ////////////////////////////////////////////////////////////////////////////

    bool SDCard::isMounted() const
    {
        return (_card != nullptr);
    }

    const char *SDCard::getMountPoint() const
    {
        return _impl->mountpoint();
    }

    uint64_t SDCard::getTotalBytes() const
    {
        FATFS *fs;
        DWORD free_clust;
        if (f_getfree("0:", &free_clust, &fs) != FR_OK) return 0;
        uint64_t tota_sect = (fs->n_fatent - 2) * fs->csize;
        uint64_t sect_size = fs->ssize;
        return (tota_sect * sect_size);
    }

    uint64_t SDCard::getUsedBytes() const
    {
        FATFS *fs;
        DWORD free_clust;
        if (f_getfree("0:", &free_clust, &fs) != FR_OK) return 0;
        uint64_t used_sect = (fs->n_fatent - 2 - free_clust) * fs->csize;
        uint64_t sect_size = fs->ssize;
        return (used_sect * sect_size);
    }

    ////////////////////////////////////////////////////////////////////////////

    bool SDCard::writeSectors(uint8_t *src, size_t startSector, size_t sectorCount)
    {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        esp_err_t res = sdmmc_write_sectors(_card, src, startSector, sectorCount);
        xSemaphoreGive(_mutex);
        return (res == ESP_OK);
    }

    bool SDCard::readSectors(uint8_t *dst, size_t startSector, size_t sectorCount)
    {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        esp_err_t res = sdmmc_read_sectors(_card, dst, startSector, sectorCount);
        xSemaphoreGive(_mutex);
        return (res == ESP_OK);
    }

    ////////////////////////////////////////////////////////////////////////////

    SDCard sdcard;
}
