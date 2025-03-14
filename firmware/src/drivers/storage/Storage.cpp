#include <USB.h>
#include "Flash.h"
#include "SDCard.h"
#include "Storage.h"
#include "../onboard/SelfReboot.h"

namespace driver
{
    class InvalidFS final : public FatFS
    {
    public:
        uint64_t sectorCount() const override { return 0; };
        uint64_t sectorSize() const override { return 0; };
        bool isMounted() const override { return false; };
    } s_invalidFS;

    Storage::Storage()
        : m_type(Type::None)
        , m_runMSC(false)
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
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return Type::SDCard;
            else if (isFlashStorage()) return Type::Flash;
        }
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
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return sdcard.sectorSize();
            else if (isFlashStorage()) return flash.sectorCount();
        }
        return 0;
    }

    uint64_t Storage::getSectorCount() const
    {
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return sdcard.sectorCount();
            else if (isFlashStorage()) return flash.sectorCount();
        }
        return 0;
    }

    uint64_t Storage::getPartitionSize() const
    {
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return sdcard.partitionSize();
            else if (isFlashStorage()) return flash.partitionSize();
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    FatFS& Storage::getFS() const
    {
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return sdcard;
            else if (isFlashStorage()) return flash;
        }
        return s_invalidFS;
    }

    const char* Storage::getFSMountPoint() const
    {
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return sdcard.mountPoint();
            else if (isFlashStorage()) return flash.mountPoint();
        }
        return nullptr;
    }

    uint64_t Storage::getFSTotalBytes() const
    {
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return sdcard.totalBytes();
            else if (isFlashStorage()) return flash.totalBytes();
        }
        return 0;
    }

    uint64_t Storage::getFSUsedBytes() const
    {
        if (!isMSCRunning())
        {
            if (isSDCardStorage()) return sdcard.usedBytes();
            else if (isFlashStorage()) return flash.usedBytes();
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
        if (!m_runMSC && (isSDCardStorage() || isFlashStorage()))
        {
            m_runMSC = USB.begin();
            m_usbMSC.vendorID(STORAGE_MSC_VENDORID);
            m_usbMSC.productID(STORAGE_MSC_PRODUCTID);
            m_usbMSC.productRevision(STORAGE_MSC_PRODUCTREV);
            m_usbMSC.mediaPresent(true);
            m_usbMSC.onStartStop([](uint8_t power_condition, bool start, bool load_eject) -> bool
            {
                if (!start && load_eject) storage.onStopMSC();
                return true;
            });

            if (isSDCardStorage())
            {
                // Mass Storage Class device for SD Card FAT
                m_usbMSC.onRead([](uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) -> int32_t
                {
                    sdcard.readSectors((uint8_t *)buffer, lba, bufsize / sdcard.sectorSize());
                    return bufsize;
                });
                m_usbMSC.onWrite([](uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) -> int32_t
                {
                    sdcard.writeSectors(buffer, lba, bufsize / sdcard.sectorSize());
                    return bufsize;
                });
                m_runMSC &= m_usbMSC.begin(sdcard.sectorCount(), sdcard.sectorSize());
            }
            else
            {
                // Mass Storage Class device for Flash FAT
                m_usbMSC.onRead([](uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) -> int32_t
                {
                    flash.readBuffer(lba, offset, buffer, bufsize);
                    return bufsize;
                });
                m_usbMSC.onWrite([](uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) -> int32_t
                {
                    flash.writeBuffer(lba, offset, buffer, bufsize);
                    return bufsize;
                });
                m_runMSC &= m_usbMSC.begin(flash.sectorCount(), flash.sectorSize());
            }
        }
        return m_runMSC;
    }

    void Storage::onStopMSC()
    {
        if (m_runMSC)
        {
            m_usbMSC.end();
            m_runMSC = false;
            driver::selfReboot.reboot();
        }
    }

    bool Storage::isMSCRunning() const
    {
        return m_runMSC;
    }

    ////////////////////////////////////////////////////////////////////////////

    bool Storage::initFlashStorage()
    {
        if (flash.begin(Flash::DEFAULT_MOUNT_POINT))
        {
            m_type = Type::Flash;
            return true;
        }
        m_type = Type::None;
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
            m_type = Type::SDCard;
            return true;
        }
    #endif
        m_type = Type::None;
        return false;
    }

    bool Storage::isFlashStorage() const
    {
        return (m_type == Type::Flash && flash.isMounted());
    }

    bool Storage::isSDCardStorage() const
    {
        return (m_type == Type::SDCard && sdcard.isMounted());
    }

    ////////////////////////////////////////////////////////////////////////////

    Storage storage;
}