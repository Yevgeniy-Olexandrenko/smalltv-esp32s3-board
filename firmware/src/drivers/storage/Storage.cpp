#include <USB.h>
#include "Flash.h"
#include "SDCard.h"
#include "Storage.h"
#include "firmware.h"

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

    uint64_t Storage::getPartitionSize() const
    {
        if (isSDCardStorage()) return sdcard.getPartitionSize();
        else if (isFlashStorage()) return flash.getPartitionSize();
        return 0;
    }

    size_t Storage::getSectorCount() const
    {
        if (isSDCardStorage()) return sdcard.getSectorCount();
        else if (isFlashStorage()) return flash.getSectorCount();
        return 0;
    }

    size_t Storage::getSectorSize() const
    {
        if (isSDCardStorage()) return sdcard.getSectorSize();
        else if (isFlashStorage()) return flash.getSectorCount();
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    const char* Storage::getFSMountPoint() const
    {
        if (isSDCardStorage()) return sdcard.getMountPoint();
        else if (isFlashStorage()) return flash.getMountPoint();
        return nullptr;
    }

    uint64_t Storage::getFSTotalBytes() const
    {
        if (isSDCardStorage()) return sdcard.getTotalBytes();
        else if (isFlashStorage()) return flash.getTotalBytes();
        return 0;
    }

    uint64_t Storage::getFSUsedBytes() const
    {
        if (isSDCardStorage()) return sdcard.getUsedBytes();
        else if (isFlashStorage()) return flash.getUsedBytes();
        return 0;
    }

    uint64_t Storage::getFSFreeBytes() const
    {
        return (getFSTotalBytes() - getFSUsedBytes());
    }

    fs::FS& Storage::getFS() const
    {
        if (isSDCardStorage()) return sdcard;
        else if (isFlashStorage()) return flash;
        return _invalidFS;
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
        if (sdcard.begin(SDCard::DEFAULT_MOUNT_POINT,
            SDCARD_CLK, SDCARD_CMD, SDCARD_D0,
            SDCARD_D1,  SDCARD_D2,  SDCARD_D3))
        {
            _type = Type::SDCard;
            return true;
        }
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