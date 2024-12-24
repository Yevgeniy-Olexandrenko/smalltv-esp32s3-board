// #pragma once

// #include <SD_MMC.h>
// #include <LittleFS.h>

// #define SDCARD_MMC_DET GPIO_NUM_47
// #define SDCARD_MMC_CLK GPIO_NUM_41
// #define SDCARD_MMC_CMD GPIO_NUM_38
// #define SDCARD_MMC_D0  GPIO_NUM_42
// #define SDCARD_MMC_D1  GPIO_NUM_21
// #define SDCARD_MMC_D2  GPIO_NUM_40
// #define SDCARD_MMC_D3  GPIO_NUM_39

// #define SDCARD_CLASS fs::SDMMCFS
// #define SPIFFS_CLASS fs::LittleFSFS 

// namespace driver
// {
//     class Storage : public fs::FS
//     {
//     public:
//         Storage();
//         void begin();

//         SDCARD_CLASS& sdcard() const;
//         SPIFFS_CLASS& spiffs() const;

//         bool isSDCardInserted() const;
//         bool isSDCardAvailable() const;
//         sdcard_type_t getSDCardType() const;
//         float getSDCardSizeMB() const;
//         float getSDCardSizeGB() const;

//         uint64_t getTotalBytes();
//         uint64_t getUsedBytes();
//     };

//     extern Storage storage;
// }
