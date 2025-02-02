#include <ff.h>
#include <USB.h>
#include "Flash.h"
#include "SDCard.h"
#include "Storage.h"
#include "defines.h"

namespace driver
{
    fs::FS _invalidFS(nullptr);

    Storage::Storage()
        : _type(Type::None)
        , _runMSC(false)
    {}

    Storage::~Storage()
    {
        end();
    }

    ////////////////////////////////////////////////////////////////////////////

    void Storage::begin(Type type)
    {
        switch (type)
        {
            case Type::Auto:
                if (!initSDCardStorage())
                {
                    initFlashStorage();
                }
                break;

            case Type::SDCard:
                initSDCardStorage();
                break;

            case Type::Flash:
                initFlashStorage();
                break;
        }
    }

    void Storage::end()
    {
        if (isSDCardStorage()) sdcard.end();
        else if (isFlashStorage()) flash.end();
    }

    ////////////////////////////////////////////////////////////////////////////

    Storage::Type Storage::getType() const
    {
        if (isSDCardStorage()) return Type::SDCard;
        else if (isFlashStorage()) return Type::Flash;
        return Type::None;
    }

    bool Storage::isLarge() const
    {
        return (getType() == Type::SDCard);
    }

    bool Storage::isFast() const
    {
        return (getType() == Type::Flash);
    }

    uint64_t Storage::getSectorSize() const
    {
        if (isSDCardStorage()) return sdcard.getSectorSize();
        else if (isFlashStorage()) return flash.getSectorCount();
        return 0;
    }

    uint64_t Storage::getSectorCount() const
    {
        if (isSDCardStorage()) return sdcard.getSectorCount();
        else if (isFlashStorage()) return flash.getSectorCount();
        return 0;
    }

    uint64_t Storage::getPartitionSize() const
    {
        if (isSDCardStorage()) return sdcard.getPartitionSize();
        else if (isFlashStorage()) return flash.getPartitionSize();
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    fs::FS& Storage::getFS() const
    {
        if (isSDCardStorage()) return sdcard;
        else if (isFlashStorage()) return flash;
        return _invalidFS;
    }

    const char* Storage::getFSMountPoint() const
    {
        if (isSDCardStorage()) return sdcard.getMountPoint();
        else if (isFlashStorage()) return flash.getMountPoint();
        return nullptr;
    }

    uint64_t Storage::getFSTotalBytes() const
    {
        int pdrv = -1;
        if (isSDCardStorage()) pdrv = sdcard.getDriveNumber();
        else if (isFlashStorage()) pdrv = flash.getDriveNumber();
        if (pdrv >= 0)
        {
            FATFS *fs; uint32_t free_clust;
            char drv[3] = { char('0' + pdrv), ':', '\0' };
            if (f_getfree(drv, &free_clust, &fs) == FR_OK)
            {
                auto total_sect = (fs->n_fatent - 2) * fs->csize;
                auto sect_size = fs->ssize;
                return (total_sect * sect_size);
            }
        }
        return 0;
    }

    uint64_t Storage::getFSUsedBytes() const
    {
        int pdrv = -1;
        if (isSDCardStorage()) pdrv = sdcard.getDriveNumber();
        else if (isFlashStorage()) pdrv = flash.getDriveNumber();
        if (pdrv >= 0)
        {
            FATFS *fs; uint32_t free_clust;
            char drv[3] = { char('0' + pdrv), ':', '\0' };
            if (f_getfree(drv, &free_clust, &fs) == FR_OK)
            {
                auto used_sect = (fs->n_fatent - 2 - free_clust) * fs->csize;
                auto sect_size = fs->ssize;
                return (used_sect * sect_size);
            }
        }
        return 0;
    }

    uint64_t Storage::getFSFreeBytes() const
    {
        return (getFSTotalBytes() - getFSUsedBytes());
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Storage::startMSC()
    {
        if (!_runMSC && (isSDCardStorage() || isFlashStorage()))
        {
            _runMSC = USB.begin();
            _usbMSC.vendorID(STORAGE_MSC_VENDORID);
            _usbMSC.productID(STORAGE_MSC_PRODUCTID);
            _usbMSC.productRevision(STORAGE_MSC_PRODUCTREV);
            _usbMSC.mediaPresent(true);
            _usbMSC.onStartStop([](uint8_t power_condition, bool start, bool load_eject) -> bool
            {
                if (!start && load_eject) storage.onStopMSC();
                return true;
            });

            if (isSDCardStorage())
            {
                // Mass Storage Class device for SD Card FAT
                _usbMSC.onRead([](uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) -> int32_t
                {
                    sdcard.readSectors((uint8_t *)buffer, lba, bufsize / sdcard.getSectorSize());
                    return bufsize;
                });
                _usbMSC.onWrite([](uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) -> int32_t
                {
                    sdcard.writeSectors(buffer, lba, bufsize / sdcard.getSectorSize());
                    return bufsize;
                });
                _runMSC &= _usbMSC.begin(sdcard.getSectorCount(), sdcard.getSectorSize());
            }
            else
            {
                // Mass Storage Class device for Flash FAT
                _usbMSC.onRead([](uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) -> int32_t
                {
                    flash.readBuffer(lba, offset, buffer, bufsize);
                    return bufsize;
                });
                _usbMSC.onWrite([](uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) -> int32_t
                {
                    flash.writeBuffer(lba, offset, buffer, bufsize);
                    return bufsize;
                });
                _runMSC &= _usbMSC.begin(flash.getSectorCount(), flash.getSectorSize());
            }
        }
        return _runMSC;
    }

    void Storage::onStopMSC()
    {
        if (_runMSC)
        {
            _runMSC = false;
            _usbMSC.end();
        }
    }

    bool Storage::isMSCRunning() const
    {
        return _runMSC;
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Storage::initFlashStorage()
    {
        if (flash.begin(Flash::DEFAULT_MOUNT_POINT))
        {
            _type = Type::Flash;
            return true;
        }
        _type = Type::None;
        return false;
    }

    bool Storage::initSDCardStorage()
    {
    #ifndef NO_SDCARD
    #if defined(SDCARD_SPI)
        if (sdcard.begin(SDCard::DEFAULT_MOUNT_POINT, 
            PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CLK, PIN_SD_CS))
    #elif defined(SDCARD_SDIO1)
        if (sdcard.begin(SDCard::DEFAULT_MOUNT_POINT,
            PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0))
    #else // SDCARD_SDIO4
        if (sdcard.begin(SDCard::DEFAULT_MOUNT_POINT,
            PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0, PIN_SD_D1, PIN_SD_D2, PIN_SD_D3))
    #endif
        {
            _type = Type::SDCard;
            return true;
        }
    #endif
        _type = Type::None;
        return false;
    }

    bool Storage::isFlashStorage() const
    {
        return (_type == Type::Flash && flash.isMounted());
    }

    bool Storage::isSDCardStorage() const
    {
        return (_type == Type::SDCard && sdcard.isMounted());
    }

    ////////////////////////////////////////////////////////////////////////////

    Storage storage;
}