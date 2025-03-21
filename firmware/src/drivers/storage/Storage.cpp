#include <USB.h>
#include "Flash.h"
#include "SDCard.h"
#include "Storage.h"
#include "drivers/onboard/SelfReboot.h"

namespace driver
{
    Storage::Storage()
        : m_type(Type::None)
        , m_fatFS(nullptr)
        , m_runMSC(false)
    {
    }

    Storage::~Storage()
    {
        end();
    }

    void Storage::begin(Type type)
    {
        USB.begin();
        if (!m_runMSC && !m_fatFS)
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
        if (!m_runMSC && m_fatFS)
        {
            delete m_fatFS;
            m_fatFS = nullptr;
        }   
    }

    Storage::Type Storage::getType() const
    {
        if (!m_runMSC && m_fatFS) return m_type;
        return Type::None;
    }

    FatFS& Storage::getFS() const
    {
        if (!m_runMSC && m_fatFS) return *m_fatFS;
        return const_cast<FatFS&>(m_invFS);
    }

    void Storage::startMSC()
    {
        if (!m_runMSC && m_fatFS)
        {
            m_usbMSC.vendorID(STORAGE_MSC_VENDORID);
            m_usbMSC.productID(STORAGE_MSC_PRODUCTID);
            m_usbMSC.productRevision(STORAGE_MSC_PRODUCTREV);
            m_usbMSC.mediaPresent(true);

            m_usbMSC.onStartStop(&mscOnStartStopCb);
            m_usbMSC.onRead(&mscOnReadCb);
            m_usbMSC.onWrite(&mscOnWriteCb);

            auto sectorSize = m_fatFS->sectorSize();
            auto sectorCount = m_fatFS->sectorCount();
            m_runMSC = m_usbMSC.begin(sectorCount, sectorSize);
        }
    }

    bool Storage::isUSBMounted() const
    {
        return bool(USB);
    }

    Storage::Type Storage::beginFlash()
    {
        #ifndef NO_FLASH
        Flash* flash = new Flash();
        if (flash->begin(Flash::DEFAULT_MOUNT_POINT))
        {
            m_fatFS = flash;
            return Type::Flash;
        }
        delete flash;
        #endif
        return Type::None;
    }

    Storage::Type Storage::beginSDCard()
    {
        #ifndef NO_SDCARD
        SDCard* sdcard = new SDCard();
        #if defined(SDCARD_SPI)
        if (sdcard->begin(SDCard::DEFAULT_MOUNT_POINT, 
            PIN_SD_CLK, PIN_SD_MOSI, PIN_SD_MISO, PIN_SD_CS))
        #elif defined(SDCARD_SDIO1)
        if (sdcard->begin(SDCard::DEFAULT_MOUNT_POINT,
            PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0))
        #else // SDCARD_SDIO4
        if (sdcard->begin(SDCard::DEFAULT_MOUNT_POINT,
            PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0, PIN_SD_D1, PIN_SD_D2, PIN_SD_D3))
        #endif
        {
            m_fatFS = sdcard;
            return Type::SDCard;
        }
        delete sdcard;
        #endif
        return Type::None;
    }

    bool Storage::mscOnStartStopCb(uint8_t power_condition, bool start, bool load_eject)
    {
        if (!start && load_eject && storage.m_runMSC)
        {
            storage.m_usbMSC.end();
            storage.m_runMSC = false;
            driver::selfReboot.reboot();
        }
        return true;
    }

    int32_t Storage::mscOnReadCb(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
    {
        return storage.m_fatFS->mscRdBuf(lba, offset, buffer, bufsize) ? bufsize : 0;
    }

    int32_t Storage::mscOnWriteCb(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
    {
        return storage.m_fatFS->mscWrBuf(lba, offset, buffer, bufsize) ? bufsize : 0;
    }

    Storage storage;
}
