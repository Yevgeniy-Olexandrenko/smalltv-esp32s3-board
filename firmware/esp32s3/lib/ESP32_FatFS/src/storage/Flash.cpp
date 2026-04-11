#include <esp_vfs_fat.h>
#include <diskio_wl.h>
#include "Flash.h"

namespace driver::details
{
    Flash::Flash()
        : FatFS()
        , m_wlHandle(WL_INVALID_HANDLE)
    {}

    Flash::~Flash() { end(); }

    bool Flash::begin(const char* mountPoint, const char* partitionLabel)
    {
        if (isMounted()) return true;
        log_i("Initializing flash FAT");

        const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, partitionLabel);
        if (!partition) 
        {
            log_e("No FAT partition found on flash with label: %s", partitionLabel);
            return false;
        }

        esp_vfs_fat_mount_config_t conf = 
        {
            .format_if_mount_failed = true,
            .max_files = 5,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
        };

        esp_err_t ret = esp_vfs_fat_spiflash_mount(mountPoint, partitionLabel, &conf, &m_wlHandle);
        if (ret != ESP_OK)
        {
            log_e("Mount FAT partition failed: %s", esp_err_to_name(ret));
            esp_vfs_fat_spiflash_unmount(mountPoint, m_wlHandle);
            m_wlHandle = WL_INVALID_HANDLE;
            return false;
        }

        uint8_t pdrv = ff_diskio_get_pdrv_wl(m_wlHandle);
        setMountPoint(mountPoint, pdrv);
        return true;
    }

    void Flash::end()
    {
        if (!isMounted()) return;
        esp_vfs_fat_spiflash_unmount(mountPoint(), m_wlHandle);
        m_wlHandle = WL_INVALID_HANDLE;
        resMountPoint();
    }

    uint64_t Flash::sectorCount() const
    {
        if (m_wlHandle == WL_INVALID_HANDLE) return 0;
        return (wl_size(m_wlHandle) / wl_sector_size(m_wlHandle));
    }

    uint64_t Flash::sectorSize() const
    {
        if (m_wlHandle == WL_INVALID_HANDLE) return 0;
        return wl_sector_size(m_wlHandle);
    }

    bool Flash::isMounted() const
    {
        return (m_wlHandle != WL_INVALID_HANDLE);
    }

    bool Flash::writeSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount)
    {
        const size_t dataAddr = startSector * sectorSize();
        const size_t dataSize = sectorCount * sectorSize();
        esp_err_t res = wl_erase_range(m_wlHandle, dataAddr, dataSize);
        if (res == ESP_OK) res = wl_write(m_wlHandle, dataAddr, data, dataSize);
        return (res == ESP_OK);
    }

    bool Flash::readSectors(uint8_t* data, uint32_t startSector, uint32_t sectorCount)
    {
        const size_t dataAddr = startSector * sectorSize();
        const size_t dataSize = sectorCount * sectorSize();
        esp_err_t res = wl_read(m_wlHandle, dataAddr, data, dataSize);
        return (res == ESP_OK);
    }
}
