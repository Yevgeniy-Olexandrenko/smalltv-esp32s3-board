#include <vfs_api.h>
#include <esp_vfs_fat.h>
#include <diskio_wl.h>

#include "Flash.h"
#include "shared/tasks/LockGuard.h"

namespace driver
{
    Flash::Flash()
        : fs::FS(FSImplPtr(new VFSImpl()))
        , m_wlHandle(WL_INVALID_HANDLE)
    {}

    Flash::~Flash() 
    { 
        end();
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Flash::begin(const char* mountPoint, const char* partitionLabel)
    {
        if (isMounted()) return true;

        task::LockGuard lock(m_mutex);
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

        _impl->mountpoint(mountPoint);
        return true;
    }

    void Flash::end()
    {
        if (!isMounted()) return;
        
        task::LockGuard lock(m_mutex);
        esp_vfs_fat_spiflash_unmount(_impl->mountpoint(), m_wlHandle);
        m_wlHandle = WL_INVALID_HANDLE;
        _impl->mountpoint(nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////

    size_t Flash::getSectorSize() const
    {
        if (m_wlHandle == WL_INVALID_HANDLE) return 0;
        return wl_sector_size(m_wlHandle);
    }

    size_t Flash::getSectorCount() const
    {
        if (m_wlHandle == WL_INVALID_HANDLE) return 0;
        return (wl_size(m_wlHandle) / wl_sector_size(m_wlHandle));
    }

    size_t Flash::getPartitionSize() const
    {
        return (getSectorSize() * getSectorCount());
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Flash::isMounted() const
    {
        return (m_wlHandle != WL_INVALID_HANDLE);
    }

    const char *Flash::getMountPoint() const
    {
        return _impl->mountpoint();
    }

    int Flash::getDriveNumber() const
    {
        return ff_diskio_get_pdrv_wl(m_wlHandle);
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Flash::writeBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
    {
        task::LockGuard lock(m_mutex);
        size_t start_addr = (wl_sector_size(m_wlHandle) * lba + offset);
        esp_err_t res = wl_erase_range(m_wlHandle, start_addr, bufsize);
        if (res == ESP_OK) res = wl_write(m_wlHandle, start_addr, buffer, bufsize);
        return (res == ESP_OK);
    }

    bool Flash::readBuffer(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
    {
        task::LockGuard lock(m_mutex);
        size_t start_addr = (wl_sector_size(m_wlHandle) * lba + offset);
        esp_err_t res = wl_read(m_wlHandle, start_addr, buffer, bufsize);
        return (res == ESP_OK);
    }

    ////////////////////////////////////////////////////////////////////////////

    Flash flash;
}
