#pragma once

#include <USB.h>
#include <USBMSC.h>
#include "Storage/Flash.h"
#include "Storage/SDCard.h"

namespace driver
{
    class Storage
    {
        using FSPtr = std::unique_ptr<details::FatFS>;

    public:
        enum class Type { None, Flash, SDCard, Auto };

        Storage();
        ~Storage();

        void begin(Type type);
        void end();

        Type getType() const;
        details::FatFS& getFS() const;
        details::FatFS& getFlashFS() const;
        details::FatFS& getSDCardFS() const;

        void startMSC();
        bool isMSCRunning() const { return m_runMSC; }
        bool isUSBMounted() const { return bool(USB); }

    private:
        details::FatFS* fatFS();

    private:
        Type m_type;
        mutable FSPtr m_flashFS;
        mutable FSPtr m_sdcardFS;
        USBMSC m_usbMSC;
        bool m_runMSC;
    };

    extern Storage storage;
}
