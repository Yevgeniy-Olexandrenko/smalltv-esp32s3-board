#include <vfs_api.h>
#include <ff.h>
#include "FatFS.h"

namespace driver
{
    FatFS::FatFS()
        : fs::FS(FSImplPtr(new VFSImpl()))
    {}

    uint64_t FatFS::partitionSize() const
    {
        return sectorSize() * sectorCount();
    }

    const char* FatFS::mountPoint() const 
    { 
        return _impl->mountpoint(); 
    }

    uint64_t FatFS::totalBytes() const
    {
        if (isMounted())
        {
            DWORD freeClusters; FATFS* fatfs;
            if (f_getfree(mountPoint(), &freeClusters, &fatfs) == FR_OK)
            {
                uint64_t totalClusters = fatfs->n_fatent - 2;
                uint64_t totalBytes = totalClusters * fatfs->csize * sectorSize();
                return totalBytes;
            }
        }
        return 0;
    }

    uint64_t FatFS::usedBytes() const
    {
        if (isMounted())
        {
            DWORD freeClusters; FATFS* fatfs;
            if (f_getfree(mountPoint(), &freeClusters, &fatfs) == FR_OK)
            {
                uint64_t totalClusters = fatfs->n_fatent - 2;
                uint64_t totalBytes = totalClusters * fatfs->csize * sectorSize();
                uint64_t freeBytes = uint64_t(freeClusters) * fatfs->csize * sectorSize();
                return totalBytes - freeBytes;
            }
        }
        return 0;
    }

    void FatFS::setMountPoint(const char* mountPoint) 
    { 
        _impl->mountpoint(mountPoint); 
    }

    void FatFS::resMountPoint() 
    { 
        _impl->mountpoint(nullptr); 
    }
}
