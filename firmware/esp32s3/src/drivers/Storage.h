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
        fatfs::FatFS& MSC_FS() const;

    private:
        Type m_type;
        mutable fatfs::FatFS* m_flashFS;
        mutable fatfs::FatFS* m_sdcardFS;
        USBMSC m_usbMSC;
        bool m_runMSC;
    };

    extern Storage storage;
}
