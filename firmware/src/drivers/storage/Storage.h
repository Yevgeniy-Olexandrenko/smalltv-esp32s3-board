#pragma once

#include <FS.h>
#include <USBMSC.h>

#define SDCARD_CLK GPIO_NUM_41
#define SDCARD_CMD GPIO_NUM_38
#define SDCARD_D0  GPIO_NUM_42
#define SDCARD_D1  GPIO_NUM_21
#define SDCARD_D2  GPIO_NUM_40
#define SDCARD_D3  GPIO_NUM_39

namespace driver
{
    class Storage
    {
    public:
        enum class Type { None, Flash, SDCard, Auto };

        Storage();
        ~Storage();

        // start and stop the storage
        void begin(Type type);
        void end();

        // get storage properties
        Type getType() const;
        bool isLarge() const;
        uint64_t getPartitionSize() const;
        size_t getSectorCount() const;
        size_t getSectorSize() const;

        // get file system and its properties 
        const char* getFSMountPoint() const;
        uint64_t getFSTotalBytes() const;
        uint64_t getFSUsedBytes() const;
        uint64_t getFSFreeBytes() const;
        fs::FS& getFS() const;
        
        // start Mass Storage Controller
        bool startMSC();
        void onStopMSC();
        bool isMSCRunning() const;

    private:
        bool initFlashStorage();
        bool initSDCardStorage();
        bool isFlashStorage() const;
        bool isSDCardStorage() const;

    private:
        Type _type;
        bool _runMSC;
      USBMSC _usbMSC;
    };

    extern Storage storage;
}