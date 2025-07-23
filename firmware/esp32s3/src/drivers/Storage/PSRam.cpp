#ifndef NO_PSRAM

#include <esp_vfs_fat.h>
#include "fatfs/vfs_fat_psram.h"
#include "fatfs/diskio_psram.h"
#include "PSRam.h"

namespace driver::details
{
    PSRam::PSRam()
        : FatFS()
        , m_psram(nullptr)
    {}

    PSRam::~PSRam() { end(); }

    bool PSRam::begin(const char* mountPoint, float portion)
    {
        if (portion > 0.f && portion < 1.f)
        {
            auto sectors = uint32_t(portion * ESP.getFreePsram()) / PSRAM_SEC_SIZE;
            return begin(mountPoint, sectors);
        }
        return false;
    }

    bool PSRam::begin(const char* mountPoint, uint32_t sectors)
    {
        if (isMounted()) return true;
        log_i("Initializing psram FAT");

        psram_partition_t* psram = nullptr;
        if (sectors >= 128)
        {
            // minimal required partition size is:
            // 128 sectors x 512 bytes = 65536 bytes
            psram = psram_partition_create(sectors);
        }
        if (!psram) 
        {
            log_e("Could not create FAT partition in PSRAM with sectors: %d", sectors);
            return false;
        }

        esp_vfs_fat_mount_config_t conf = 
        {
            .format_if_mount_failed = true,
            .max_files = 5,
            .allocation_unit_size = PSRAM_SEC_SIZE
        };

        esp_err_t ret = esp_vfs_fat_psram_mount(mountPoint, psram, &conf);
        if (ret != ESP_OK)
        {
            log_e("Mount FAT partition failed: %s", esp_err_to_name(ret));
            esp_vfs_fat_psram_unmount(mountPoint, psram);
            psram_partition_delete(psram);
            return false;
        }

        uint8_t pdrv = ff_diskio_get_pdrv_psram(psram);
        setMountPoint(mountPoint, pdrv);
        m_psram = psram;
        return true;
    }

    void PSRam::end()
    {
        if (!isMounted()) return;
        esp_vfs_fat_psram_unmount(mountPoint(), m_psram);
        psram_partition_delete(m_psram);
        m_psram = nullptr;
        resMountPoint();
    }

    uint64_t PSRam::sectorCount() const
    {
        return (m_psram ? m_psram->sectors : 0);
    }

    uint64_t PSRam::sectorSize() const
    {
        return (m_psram ? PSRAM_SEC_SIZE : 0);
    }

    bool PSRam::isMounted() const
    {
        return (m_psram != nullptr);
    }

    bool PSRam::writeSectors(uint8_t *data, uint32_t startSector, uint32_t sectorCount)
    {
        esp_err_t res = ff_diskio_write_sectors_psram(m_psram, data, startSector, sectorCount);
        return (res == ESP_OK);
    }

    bool PSRam::readSectors(uint8_t *data, uint32_t startSector, uint32_t sectorCount)
    {
        esp_err_t res = ff_diskio_read_sectors_psram(m_psram, data, startSector, sectorCount);
        return (res == ESP_OK);
    }
}
#endif
