#pragma once

#include <USB.h>
#include <USBMSC.h>
#include "Storage/FatFS.h"
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
        bool isLarge() const { return (getType() == Type::SDCard); }
        bool isFast()  const { return (getType() == Type::Flash ); }
        details::FatFS& getFS() const;

        void startMSC();
        bool isMSCRunning() const { return m_runMSC; }
        bool isUSBMounted() const { return bool(USB); }

    private:
        Type beginFlash();
        Type beginSDCard();

        static bool mscOnStartStopCb(uint8_t power_condition, bool start, bool load_eject);
        static int32_t mscOnReadCb(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
        static int32_t mscOnWriteCb(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    
    private:
        Type m_type;
        FSPtr m_fsPtr;
        USBMSC m_usbMSC;
        bool m_runMSC;
    };

    extern Storage storage;
}
