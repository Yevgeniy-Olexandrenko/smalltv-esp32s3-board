#pragma once

#include <USB.h>
#include <USBMSC.h>
#include <fatfs_impl/Flash.h>
#include <fatfs_impl/SDCard.h>

namespace driver
{
    class Storage
    {
    public:
        enum class Type { None, Flash, SDCard, Auto };

        Storage();
        ~Storage();

        void begin(Type type);
        void end();

        Type getType() const;
        fatfs::FatFS& getFS() const;
        fatfs::FatFS& getFlashFS() const;
        fatfs::FatFS& getSDCardFS() const;

        void startMSC();
        bool isMSCRunning() const { return m_runMSC; }
        bool isUSBMounted() const { return bool(USB); }

    private:
        static fatfs::FatFS& mscFS();
        static bool mscOnStartStop(uint8_t power_condition, bool start, bool load_eject);
        static int  mscOnRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
        static int  mscOnWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);

    private:
        Type m_type;
        mutable bool m_flashReady;
        mutable bool m_sdcardReady;
        USBMSC m_usbMSC;
        bool m_runMSC;
    };

    extern Storage storage;
}
