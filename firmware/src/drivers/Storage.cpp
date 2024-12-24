// #include "Storage.h"
// #include <FSImpl.h>

// #define SDCARD SD_MMC
// #define SPIFFS LittleFS

// namespace driver
// {
//     using namespace fs;

//     class StorageFileImpl : public FileImpl
//     {
//     public:
//         StorageFileImpl(File& file) : m_file(file) {}
//         virtual ~StorageFileImpl() {}

//         size_t write(const uint8_t *buf, size_t size) override { return m_file.write(buf, size); }
//         size_t read(uint8_t* buf, size_t size) override { return m_file.read(buf, size); }
//         void flush() override { m_file.flush(); }
//         bool seek(uint32_t pos, SeekMode mode) override { return m_file.seek(pos, mode); }
//         size_t position() const override { return m_file.position(); }
//         size_t size() const override { return m_file.size(); }
//         bool setBufferSize(size_t size) override { return m_file.setBufferSize(size); }
//         void close() override { m_file.close(); }
//         time_t getLastWrite() override { return m_file.getLastWrite(); }
//         const char* path() const override { return m_file.path(); }
//         const char* name() const override { return m_file.name(); }
//         bool isDirectory() override { return m_file.isDirectory(); }
//         FileImplPtr openNextFile(const char* mode) override 
//         { 
//             auto file = m_file.openNextFile();
//             return FileImplPtr(new StorageFileImpl(file));
//         }
//         bool seekDir(long position) override { return m_file.seekDir(position); }
//         String getNextFileName() override { return m_file.getNextFileName(); }
//         String getNextFileName(bool *isDir) override { return m_file.getNextFileName(isDir); }
//         void rewindDirectory() override { m_file.rewindDirectory(); }
//         operator bool() override { return m_file; }

//     protected:
//         File m_file;
//     };

//     class StorageFSImpl : public FSImpl
//     {
//         friend class Storage;
//         bool m_sdcardOK;

//     public:
//         StorageFSImpl() {}
//         virtual ~StorageFSImpl() {}

//         FileImplPtr open(const char* path, const char* mode, const bool create) override
//         {
//             auto file = (m_sdcardOK ? SDCARD.open(path, mode, create) : SPIFFS.open(path, mode, create));
//             return FileImplPtr(new StorageFileImpl(file));
//         }

//         bool exists(const char* path) override
//         {
//             return (m_sdcardOK ? SDCARD.exists(path) : SPIFFS.exists(path));
//         }

//         bool rename(const char* pathFrom, const char* pathTo) override
//         {
//             return (m_sdcardOK ? SDCARD.rename(pathFrom, pathTo) : SPIFFS.rename(pathFrom, pathTo));
//         }

//         bool remove(const char* path) override
//         {
//             return (m_sdcardOK ? SDCARD.remove(path) : SPIFFS.remove(path));
//         }

//         bool mkdir(const char *path) override
//         {
//             return (m_sdcardOK ? SDCARD.mkdir(path) : SPIFFS.mkdir(path));
//         }

//         bool rmdir(const char *path) override
//         {
//             return (m_sdcardOK ? SDCARD.rmdir(path) : SPIFFS.rmdir(path));
//         }
//     };

//     Storage::Storage()
//         : fs::FS(fs::FSImplPtr(new StorageFSImpl()))
//     {}

//     void Storage::begin()
//     {
//         bool& sdcardOK = static_cast<StorageFSImpl&>(*_impl).m_sdcardOK;

//         // sdcard init
//         sdcardOK = true;
//         pinMode(SDCARD_MMC_DET, INPUT_PULLUP);
//         sdcardOK &= SD_MMC.setPins(
//             SDCARD_MMC_CLK, // clock
//             SDCARD_MMC_CMD, // command
//             SDCARD_MMC_D0,  // data0
//             SDCARD_MMC_D1,  // data1
//             SDCARD_MMC_D2,  // data2
//             SDCARD_MMC_D3); // data3
//         sdcardOK &= SD_MMC.begin();
//         sdcardOK &= (SD_MMC.cardType() != CARD_NONE);

//         // spiffs init
//         LittleFS.begin(true);
//     }

//     SDCARD_CLASS &Storage::sdcard() const { return SDCARD; }
//     SPIFFS_CLASS &Storage::spiffs() const { return SPIFFS; }

//     bool Storage::isSDCardInserted() const
//     {
//         return (digitalRead(SDCARD_MMC_DET) == LOW);
//     }

//     bool Storage::isSDCardAvailable() const
//     {
//         return static_cast<StorageFSImpl&>(*_impl).m_sdcardOK;
//     }

//     sdcard_type_t Storage::getSDCardType() const
//     {
//         return SD_MMC.cardType();
//     }

//     float Storage::getSDCardSizeMB() const
//     {
//         return int(SD_MMC.cardSize() / (1024 * 1024));
//     }

//     float Storage::getSDCardSizeGB() const
//     {
//         return float(getSDCardSizeMB() / 1024);
//     }

//     uint64_t Storage::getTotalBytes()
//     {
//         return (isSDCardAvailable() ? SDCARD.totalBytes() : SPIFFS.totalBytes());
//     }

//     uint64_t Storage::getUsedBytes()
//     {
//         return (isSDCardAvailable() ? SDCARD.usedBytes() : SPIFFS.usedBytes());
//     }

//     Storage storage;
// }
