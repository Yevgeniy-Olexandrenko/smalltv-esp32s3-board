#ifndef NO_SDCARD

#include <esp_vfs_fat.h>
#include <driver/sdmmc_defs.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include <sdmmc_cmd.h>
#include <diskio_sdmmc.h>
#include "SDCard.h"

namespace driver
{
    SDCard::SDCard()
        : FatFS()
        , m_spiSlot(-1)
        , m_oneBitMode(false)
        , m_card(nullptr)
    {}

    SDCard::~SDCard() { end(); }

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

        m_oneBitMode = (d1 == GPIO_NUM_NC || d2 == GPIO_NUM_NC || d3 == GPIO_NUM_NC);
        sdmmc_host_t m_host = SDMMC_HOST_DEFAULT();
        m_host.max_freq_khz = SDMMC_FREQ_52M;
        m_host.flags = (m_oneBitMode ? SDMMC_HOST_FLAG_1BIT : SDMMC_HOST_FLAG_4BIT);

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        #ifdef SOC_SDMMC_USE_GPIO_MATRIX
        slot_config.width = (m_oneBitMode ? 1 : 4);
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

        esp_err_t ret = esp_vfs_fat_sdmmc_mount(mountPoint, &m_host, &slot_config, &mount_config, &m_card);
        if (ret != ESP_OK)
        {
            log_e("Initializing SD card failed: %s\n", esp_err_to_name(ret));
            m_card = nullptr;
            return false;
        }

        log_i("SD card mounted at: %s\n", mountPoint);
        sdmmc_card_print_info(stdout, m_card);
        
        setMountPoint(mountPoint);
        return true;
    }

    // SPI mode
    bool SDCard::begin(
        const char *mountPoint,
        gpio_num_t clk,
        gpio_num_t mosi,
        gpio_num_t miso,
        gpio_num_t cs)
    {
        if (isMounted()) return true;
        log_i("Initializing SD card");

        sdmmc_host_t m_host = SDSPI_HOST_DEFAULT();
        m_spiSlot = m_host.slot = SPI3_HOST;

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

        esp_err_t ret = spi_bus_initialize(spi_host_device_t(m_spiSlot), &bus_cfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK) 
        {
            log_e("Initializing SPI bus failed: %s\n", esp_err_to_name(ret));
            return false;
        }

        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.host_id = spi_host_device_t(m_spiSlot);
        slot_config.gpio_cs = cs;

        esp_vfs_fat_mount_config_t mount_config = 
        {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 0
        };

        ret = esp_vfs_fat_sdspi_mount(mountPoint, &m_host, &slot_config, &mount_config, &m_card);
        if (ret != ESP_OK)
        {
            log_e("Initializing SD card failed: %s\n", esp_err_to_name(ret));
            spi_bus_free(spi_host_device_t(m_spiSlot));
            m_card = nullptr;
            m_spiSlot = -1;
            return false;
        }

        log_i("SD card mounted at: %s\n", mountPoint);
        sdmmc_card_print_info(stdout, m_card);
        
        setMountPoint(mountPoint);
        return true;
    }

    void SDCard::end()
    {
        if (!isMounted()) return; 
        esp_vfs_fat_sdcard_unmount(mountPoint(), m_card);
        spi_bus_free(spi_host_device_t(m_spiSlot));
        resMountPoint();
        m_card = nullptr;
        m_spiSlot = -1; 
    }

    SDCard::Type SDCard::getCardType() const
    {
        if(!m_card) return Type::NONE;
        return (m_card->ocr & SD_OCR_SDHC_CAP ? Type::SDHC : Type::SD);
    }

    SDCard::Interface SDCard::getCardInterface() const
    {
        if(!m_card) return Interface::NONE;
        return (m_spiSlot >= 0 ? Interface::SPI : (m_oneBitMode ? Interface::SDIO1 : Interface::SDIO4));
    }

    uint64_t SDCard::sectorCount() const
    {
        return (m_card ? m_card->csd.capacity : 0);
    }

    uint64_t SDCard::sectorSize() const
    {
        return (m_card ? m_card->csd.sector_size : 0);
    }

    bool SDCard::isMounted() const
    {
        return (m_card != nullptr);
    }

    bool SDCard::writeSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount)
    {
        esp_err_t res = sdmmc_write_sectors(m_card, data, startSector, sectorCount);
        return (res == ESP_OK);
    }

    bool SDCard::readSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount)
    {
        esp_err_t res = sdmmc_read_sectors(m_card, data, startSector, sectorCount);
        return (res == ESP_OK);
    }
}
#endif
