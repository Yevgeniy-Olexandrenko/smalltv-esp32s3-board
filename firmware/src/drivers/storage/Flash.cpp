#include <vfs_api.h>
#include <esp_vfs_fat.h>
#include <diskio_wl.h>

#include "Flash.h"
#include "shared/tasks/LockGuard.h"

namespace driver
{
    Flash::Flash()
        : fs::FS(FSImplPtr(new VFSImpl()))
        , _wl_handle(WL_INVALID_HANDLE)
    {}

    Flash::~Flash() 
    { 
        end();
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Flash::begin(const char* mountPoint, const char* partitionLabel)
    {
        if (isMounted()) return true;

        task::LockGuard lock(_mutex);
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

        esp_err_t ret = esp_vfs_fat_spiflash_mount(mountPoint, partitionLabel, &conf, &_wl_handle);
        if (ret != ESP_OK)
        {
            log_e("Mount FAT partition failed: %s", esp_err_to_name(ret));
            esp_vfs_fat_spiflash_unmount(mountPoint, _wl_handle);
            _wl_handle = WL_INVALID_HANDLE;
            return false;
        }

        _impl->mountpoint(mountPoint);
        return true;
    }

    void Flash::end()
    {
        if (!isMounted()) return;
        
        task::LockGuard lock(_mutex);
        esp_vfs_fat_spiflash_unmount(_impl->mountpoint(), _wl_handle);
        _wl_handle = WL_INVALID_HANDLE;
        _impl->mountpoint(nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////

    size_t Flash::getSectorSize() const
    {
        if (_wl_handle == WL_INVALID_HANDLE) return 0;
        return wl_sector_size(_wl_handle);
    }

    size_t Flash::getSectorCount() const
    {
        if (_wl_handle == WL_INVALID_HANDLE) return 0;
        return (wl_size(_wl_handle) / wl_sector_size(_wl_handle));
    }

    size_t Flash::getPartitionSize() const
    {
        return (getSectorSize() * getSectorCount());
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Flash::isMounted() const
    {
        return (_wl_handle != WL_INVALID_HANDLE);
    }

    const char *Flash::getMountPoint() const
    {
        return _impl->mountpoint();
    }

    uint64_t Flash::getTotalBytes() const
    {
        if (isMounted())
        {
            FATFS *fs; uint32_t free_clust;
            auto pdrv = ff_diskio_get_pdrv_wl(_wl_handle);
            char drv[3] = { char('0' + pdrv), ':', '\0' };
            if (f_getfree(drv, &free_clust, &fs) == FR_OK)
            {
                auto total_sect = (fs->n_fatent - 2) * fs->csize;
                auto sect_size = CONFIG_WL_SECTOR_SIZE; // fs->ssize
                return (total_sect * sect_size);
            }
        }
        return 0;
    }

    uint64_t Flash::getUsedBytes() const
    {
        if (isMounted())
        {
            FATFS *fs; uint32_t free_clust;
            auto pdrv = ff_diskio_get_pdrv_wl(_wl_handle);
            char drv[3] = { char('0' + pdrv), ':', '\0' };
            if (f_getfree(drv, &free_clust, &fs) == FR_OK)
            {
                auto used_sect = (fs->n_fatent - 2 - free_clust) * fs->csize;
                auto sect_size = CONFIG_WL_SECTOR_SIZE; // fs->ssize
                return (used_sect * sect_size);
            }
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Flash::writeBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
    {
        task::LockGuard lock(_mutex);
        size_t start_addr = (wl_sector_size(_wl_handle) * lba + offset);
        esp_err_t res = wl_erase_range(_wl_handle, start_addr, bufsize);
        if (res == ESP_OK) res = wl_write(_wl_handle, start_addr, buffer, bufsize);
        return (res == ESP_OK);
    }

    bool Flash::readBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
    {
        task::LockGuard lock(_mutex);
        size_t start_addr = (wl_sector_size(_wl_handle) * lba + offset);
        esp_err_t res = wl_read(_wl_handle, start_addr, buffer, bufsize);
        return (res == ESP_OK);
    }

    ////////////////////////////////////////////////////////////////////////////

    Flash flash;
}
