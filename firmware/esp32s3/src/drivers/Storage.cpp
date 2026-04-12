#include "Storage.h"
#include "SelfReboot.h"
#include "hardware/board.h"
#include "firmware/defines.h"

namespace driver
{
    Storage::Storage()
        : m_type(Type::None)
        , m_flashReady(false)
        , m_sdcardReady(false)
        , m_runMSC(false)
    {}

    Storage::~Storage() { end(); }

    void Storage::begin(Type type)
    {
        if (!m_runMSC && m_type == Type::None)
        {
            if ((type == Type::Auto || type == Type::SDCard) && getSDCardFS().isMounted())
                m_type = Type::SDCard;
            else if ((type == Type::Auto || type == Type::Flash) && getFlashFS().isMounted())
                m_type = Type::Flash;
        }
    }

    void Storage::end()
    {
        if (!m_runMSC)
        {
            fatfs::flash.end();
            m_flashReady = false;
            fatfs::sdcard.end();
            m_sdcardReady = false;
            m_type = Type::None;
        }
    }

    Storage::Type Storage::getType() const
    {
        return (getFS().isMounted() ? m_type : Type::None);
    }

    fatfs::FatFS& Storage::getFS() const
    {
        if (m_type == Type::Flash ) return getFlashFS();
        if (m_type == Type::SDCard) return getSDCardFS();
        return fatfs::invalid;
    }

    fatfs::FatFS& Storage::getFlashFS() const
    {
        if (!m_flashReady)
        {
            fatfs::flash.begin(fatfs::Flash::DEFAULT_MOUNT_POINT);
            m_flashReady = true;
        }
        return (m_runMSC ? fatfs::invalid : fatfs::flash);
    }

    fatfs::FatFS& Storage::getSDCardFS() const
    {
        #ifndef NO_SDCARD
        if (!m_sdcardReady)
        {
            #if defined(SDCARD_SPI)
            fatfs::sdcard.begin(
                fatfs::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_SCK, PIN_SD_MOSI, PIN_SD_MISO, PIN_SD_CS);
            #elif defined(SDCARD_SDIO1)
            fatfs::sdcard.begin(
                fatfs::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
            #else // SDCARD_SDIO4
            fatfs::sdcard.begin(
                fatfs::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0, PIN_SD_D1, PIN_SD_D2, PIN_SD_D3);
            #endif
            m_sdcardReady = true;
        }
        #endif
        return (m_runMSC ? fatfs::invalid : fatfs::sdcard);
    }

    void Storage::startMSC()
    {
        fatfs::FatFS& fs = mscFS();
        if (!m_runMSC && fs.isMounted())
        {
            // configure MSC device properties
            m_usbMSC.vendorID(STORAGE_MSC_VENDORID);
            m_usbMSC.productID(STORAGE_MSC_PRODUCTID);
            m_usbMSC.productRevision(STORAGE_MSC_PRODUCTREV);
            m_usbMSC.mediaPresent(true);

            // configure MSC callbacks
            m_usbMSC.onStartStop(mscOnStartStop);
            m_usbMSC.onRead(mscOnRead);
            m_usbMSC.onWrite(mscOnWrite);

            // start MSC device over USB
            m_runMSC =  m_usbMSC.begin(fs.sectorCount(), fs.sectorSize());
            m_runMSC &= USB.begin();
        }
    }

    fatfs::FatFS& Storage::mscFS()
    {
        if (storage.m_type == Type::Flash ) return fatfs::flash;
        if (storage.m_type == Type::SDCard) return fatfs::sdcard;
        return fatfs::invalid;
    }

    bool Storage::mscOnStartStop(uint8_t power_condition, bool start, bool load_eject)
    {
        if (storage.m_runMSC && load_eject)
        {
            storage.m_usbMSC.end();
            storage.m_runMSC = false;
            driver::selfReboot.reboot();
        }
        return true;
    }

    int Storage::mscOnRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
    {
        const uint32_t sectorCount = (bufsize / storage.mscFS().sectorSize());
        return (storage.mscFS().readSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0);
    }

    int Storage::mscOnWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
    {
        const uint32_t sectorCount = (bufsize / storage.mscFS().sectorSize());
        return (storage.mscFS().writeSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0);
    }

    Storage storage;
}
