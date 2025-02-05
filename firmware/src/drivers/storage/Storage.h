#pragma once

#include <FS.h>
#include <USBMSC.h>
#include "board.h"

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

        // storage properties
        Type getType() const;
        bool isLarge() const;
        bool isFast()  const;
        uint64_t getSectorSize() const;
        uint64_t getSectorCount() const;
        uint64_t getPartitionSize() const;
        
        // file system and its properties
        fs::FS& getFS() const;
        const char* getFSMountPoint() const;
        uint64_t getFSTotalBytes() const;
        uint64_t getFSUsedBytes() const;
        uint64_t getFSFreeBytes() const;
        
        // start MSC device mode
        bool startMSC();
        void onStopMSC();
        bool isMSCRunning() const;

    private:
        bool initFlashStorage();
        bool initSDCardStorage();
        bool isFlashStorage() const;
        bool isSDCardStorage() const;

    private:
        Type m_type;
        bool m_runMSC;
      USBMSC m_usbMSC;
    };

    extern Storage storage;
}