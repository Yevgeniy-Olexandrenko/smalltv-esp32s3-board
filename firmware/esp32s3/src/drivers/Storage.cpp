#include "Storage.h"
#include "SelfReboot.h"
#include "hardware/board.h"
#include "firmware/defines.h"

namespace driver
{
    namespace details { FatFS invalidFS; }

    Storage::Storage()
        : m_type(Type::None)
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
            m_flashFS.reset();
            m_sdcardFS.reset();
            m_type = Type::None;
        }
    }

    Storage::Type Storage::getType() const
    {
        return (getFS().isMounted() ? m_type : Type::None);
    }

    details::FatFS& Storage::getFS() const
    {
        if (m_type == Type::Flash) return getFlashFS();
        if (m_type == Type::SDCard) return getSDCardFS();
        return details::invalidFS;
    }

    details::FatFS& Storage::getFlashFS() const
    {
        if (!m_flashFS)
        {
            auto flash = new details::Flash();
            m_flashFS.reset(flash);

            if (!flash->begin(details::Flash::DEFAULT_MOUNT_POINT))
            {
                // something went wrong, init
                // with invalid file system!
                m_flashFS.reset(new details::FatFS());
            }
        }
        return (m_runMSC ? details::invalidFS : (*m_flashFS));
    }

    details::FatFS &Storage::getSDCardFS() const
    {
        if (!m_sdcardFS)
        {
            #ifndef NO_SDCARD
            auto sdcard = new details::SDCard();
            m_sdcardFS.reset(sdcard);

            #if defined(SDCARD_SPI)
            if (!sdcard->begin(
                details::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_SCK, PIN_SD_MOSI, PIN_SD_MISO, PIN_SD_CS))
            #elif defined(SDCARD_SDIO1)
            if (!sdcard->begin(
                details::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0))
            #else // SDCARD_SDIO4
            if (!sdcard->begin(
                details::SDCard::DEFAULT_MOUNT_POINT,
                PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0, PIN_SD_D1, PIN_SD_D2, PIN_SD_D3))
            #endif
            #endif
            {
                // something went wrong, init
                // with invalid file system!
                m_sdcardFS.reset(new details::FatFS());
            }
        }
        return (m_runMSC ? details::invalidFS : (*m_sdcardFS));
    }

    void Storage::startMSC()
    {
        if (getFS().isMounted())
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
                const uint32_t sectorCount = (bufsize / storage.fatFS()->sectorSize());
                return (storage.fatFS()->readSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0);
            });

            // on write data to MSC device FAT sectors
            m_usbMSC.onWrite([](uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) -> int
            {
                const uint32_t sectorCount = (bufsize / storage.fatFS()->sectorSize());
                return (storage.fatFS()->writeSectors((uint8_t*)buffer, lba, sectorCount) ? bufsize : 0);
            });

            // start MSC device over USB
            m_runMSC = m_usbMSC.begin(fatFS()->sectorCount(), fatFS()->sectorSize());
            USB.begin();
        }
    }

    details::FatFS* Storage::fatFS()
    {
        details::FatFS* fatFS = nullptr;
        if (m_type == Type::Flash) fatFS = m_flashFS.get();
        if (m_type == Type::SDCard) fatFS = m_sdcardFS.get();
        return (fatFS ? fatFS : &details::invalidFS);
    }

    Storage storage;
}
