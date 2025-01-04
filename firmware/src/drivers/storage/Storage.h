#pragma once

#include <FS.h>
#include <USBMSC.h>

// sdmmc
#define SDMMC_CLK GPIO_NUM_41
#define SDMMC_CMD GPIO_NUM_38
#define SDMMC_D0  GPIO_NUM_42
#define SDMMC_D1  GPIO_NUM_21
#define SDMMC_D2  GPIO_NUM_40
#define SDMMC_D3  GPIO_NUM_39

// sdspi
#define SDSPI_MISO SDMMC_D0
#define SDSPI_MOSI SDMMC_CMD
#define SDSPI_CLK  SDMMC_CLK
#define SDSPI_CS   SDMMC_D3

// chose mmc or spi interface
#define USE_SDSPI 1

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
        Type _type;
        bool _runMSC;
      USBMSC _usbMSC;
    };

    extern Storage storage;
}