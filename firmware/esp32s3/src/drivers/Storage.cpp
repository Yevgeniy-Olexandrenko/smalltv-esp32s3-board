#include "Storage.h"
#include "SelfReboot.h"
#include "hardware/board.h"
#include "firmware/defines.h"

namespace driver
{
    Storage::Storage()
        : m_type(Type::None)
        , m_runMSC(false)
    {}

    Storage::~Storage() { end(); }

    void Storage::begin(Type type)
    {
        if (!m_runMSC && !m_fsPtr)
        {
            switch (type)
            {
                case Type::Auto:
                    m_type = beginSDCard();
                    if (m_type == Type::None)
                        m_type = beginFlash(); 
                    break;

                case Type::SDCard: 
                    m_type = beginSDCard(); 
                    break;

                case Type::Flash: 
                    m_type = beginFlash(); 
                    break;
            }
        }
    }

    void Storage::end()
    {
        if (!m_runMSC && m_fsPtr) m_fsPtr.reset();
    }

    Storage::Type Storage::getType() const
    {
        if (!m_runMSC && m_fsPtr) return m_type;
        return Type::None;
    }

    details::FatFS& Storage::getFS() const
    {
        if (!m_runMSC && m_fsPtr) return *m_fsPtr;
        static details::FatFS s_invFS;
        return s_invFS;
    }

    void Storage::startMSC()
    {
        if (!m_runMSC && m_fsPtr)
        {
            m_usbMSC.vendorID(STORAGE_MSC_VENDORID);
            m_usbMSC.productID(STORAGE_MSC_PRODUCTID);
            m_usbMSC.productRevision(STORAGE_MSC_PRODUCTREV);
            m_usbMSC.mediaPresent(true);

            m_usbMSC.onStartStop(&mscOnStartStopCb);
            m_usbMSC.onRead(&mscOnReadCb);
            m_usbMSC.onWrite(&mscOnWriteCb);

            auto sectorSize = m_fsPtr->sectorSize();
            auto sectorCount = m_fsPtr->sectorCount();
            m_runMSC = m_usbMSC.begin(sectorCount, sectorSize);
            USB.begin();
        }
    }

    Storage::Type Storage::beginFlash()
    {
        #ifndef NO_FLASH
        details::Flash* flash = new details::Flash();
        if (flash->begin(details::Flash::DEFAULT_MOUNT_POINT))
        {
            m_fsPtr.reset(flash);
            return Type::Flash;
        }
        delete flash;
        #endif
        return Type::None;
    }

    Storage::Type Storage::beginSDCard()
    {
        #ifndef NO_SDCARD
        details::SDCard* sdcard = new details::SDCard();
        #if defined(SDCARD_SPI)
        if (sdcard->begin(
            details::SDCard::DEFAULT_MOUNT_POINT,
            PIN_SD_SCK, PIN_SD_MOSI, PIN_SD_MISO, PIN_SD_CS))
        #elif defined(SDCARD_SDIO1)
        if (sdcard->begin(
            details::SDCard::DEFAULT_MOUNT_POINT,
            PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0))
        #else // SDCARD_SDIO4
        if (sdcard->begin(
            details::SDCard::DEFAULT_MOUNT_POINT,
            PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0, PIN_SD_D1, PIN_SD_D2, PIN_SD_D3))
        #endif
        {
            m_fsPtr.reset(sdcard);
            return Type::SDCard;
        }
        delete sdcard;
        #endif
        return Type::None;
    }

    bool Storage::mscOnStartStopCb(uint8_t power_condition, bool start, bool load_eject)
    {
        if (load_eject && storage.m_runMSC)
        {
            storage.m_usbMSC.end();
            storage.m_runMSC = false;
            driver::selfReboot.reboot();
        }
        return true;
    }

    int32_t Storage::mscOnReadCb(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
    {
        const uint32_t sectorCount = bufsize / storage.m_fsPtr->sectorSize();
        return storage.m_fsPtr->readSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0;
    }

    int32_t Storage::mscOnWriteCb(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
    {
        const uint32_t sectorCount = bufsize / storage.m_fsPtr->sectorSize();
        return storage.m_fsPtr->writeSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0;
    }

    Storage storage;
}
