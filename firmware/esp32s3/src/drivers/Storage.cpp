#include "Storage.h"
#include "SelfReboot.h"
#include "hardware/board.h"
#include "firmware/defines.h"

namespace driver
{
    Storage::Storage()
        : m_type(Type::None)
        , m_flashFS(nullptr)
        , m_sdcardFS(nullptr)
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
            m_type = Type::None;
            m_flashFS = m_sdcardFS = nullptr;
        }
    }

    Storage::Type Storage::getType() const
    {
        return (getFS().isMounted() ? m_type : Type::None);
    }

    fatfs::FatFS& Storage::getFS() const
    {
        if (m_type == Type::Flash) return getFlashFS();
        if (m_type == Type::SDCard) return getSDCardFS();
        return fatfs::invalid;
    }

    fatfs::FatFS& Storage::getFlashFS() const
    {
        if (!m_flashFS)
        {
            m_flashFS = &fatfs::invalid;
            if (fatfs::flash.begin(fatfs::Flash::DEFAULT_MOUNT_POINT))
            {
                m_flashFS = &fatfs::flash;
            }
        }
        return (m_runMSC ? fatfs::invalid : *m_flashFS);
    }

    fatfs::FatFS &Storage::getSDCardFS() const
    {
        if (!m_sdcardFS)
        {
            m_sdcardFS = &fatfs::invalid;

            #ifndef NO_SDCARD
            #if defined(SDCARD_SPI)
            if (fatfs::sdcard.begin(
                fatfs::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_SCK, PIN_SD_MOSI, PIN_SD_MISO, PIN_SD_CS))
            #elif defined(SDCARD_SDIO1)
            if (fatfs::sdcard.begin(
                fatfs::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0))
            #else // SDCARD_SDIO4
            if (fatfs::sdcard.begin(
                fatfs::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0, PIN_SD_D1, PIN_SD_D2, PIN_SD_D3))
            #endif
            #endif
            {
                m_sdcardFS = &fatfs::sdcard;
            }
        }
        return (m_runMSC ? fatfs::invalid : *m_sdcardFS);
    }

    void Storage::startMSC()
    {
        if (MSC_FS().isMounted())
        {
            // configure MSC device properties
            m_usbMSC.vendorID(STORAGE_MSC_VENDORID);
            m_usbMSC.productID(STORAGE_MSC_PRODUCTID);
            m_usbMSC.productRevision(STORAGE_MSC_PRODUCTREV);
            m_usbMSC.mediaPresent(true);

            // on start/stop MSC device
            m_usbMSC.onStartStop([](uint8_t power_condition, bool start, bool load_eject) -> bool
            {
                if (load_eject && storage.m_runMSC)
                {
                    storage.m_usbMSC.end();
                    storage.m_runMSC = false;
                    driver::selfReboot.reboot();
                }
                return true;
            });

            // on read data from MSC device FAT sectors
            m_usbMSC.onRead([](uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) -> int
            {
                const uint32_t sectorCount = (bufsize / storage.MSC_FS().sectorSize());
                return (storage.MSC_FS().readSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0);
            });

            // on write data to MSC device FAT sectors
            m_usbMSC.onWrite([](uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) -> int
            {
                const uint32_t sectorCount = (bufsize / storage.MSC_FS().sectorSize());
                return (storage.MSC_FS().writeSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0);
            });

            // start MSC device over USB
            m_runMSC = m_usbMSC.begin(MSC_FS().sectorCount(), MSC_FS().sectorSize());
            USB.begin();
        }
    }

    fatfs::FatFS& Storage::MSC_FS() const
    {
        fatfs::FatFS* fatFS = nullptr;
        if (m_type == Type::Flash) fatFS = m_flashFS;
        if (m_type == Type::SDCard) fatFS = m_sdcardFS;
        return (fatFS ? *fatFS : fatfs::invalid);
    }

    Storage storage;
}
