#include "SDCardMSC.h"
#include "SDCard.h"
#include <USB.h>
#include <esp_task_wdt.h>

namespace driver
{
    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
    {
        // this writes a complete sector so we should return sector size on success
        if (sdcard.writeSectors(buffer, lba, bufsize / sdcard.getSectorSize()))
        {
            return bufsize;
        }
        return bufsize;
        // return -1;
    }

    static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
    {
        // this reads a complete sector so we should return sector size on success
        if (sdcard.readSectors((uint8_t *)buffer, lba, bufsize / sdcard.getSectorSize()))
        {
            return bufsize;
        }
        return -1;
    }

    static bool onStartStop(uint8_t power_condition, bool start, bool load_eject)
    {
        if (load_eject)
        {
            sdcardmsc.end();
        }
        return true;
    }

    void SDCardMSC::begin()
    {
        if (sdcard.isMounted() && !_run)
        {
            _msc.vendorID("ESP32-S3");
            _msc.productID("SmallTV");
            _msc.productRevision("1.0");
            _msc.onRead(onRead);
            _msc.onWrite(onWrite);
            _msc.onStartStop(onStartStop);
            _msc.mediaPresent(true);

            _run = true;
            _run &= USB.begin();
            _run &= _msc.begin(sdcard.getSectorCount(), sdcard.getSectorSize());
        }
    }

    void SDCardMSC::end()
    {
        if (_run)
        {
            _run = false;
            _msc.end();

            // reset
            esp_task_wdt_init(1, true);
            esp_task_wdt_add(nullptr);
            while (true) {}
        }
    }

    bool SDCardMSC::isRunning() const
    {
        return _run;
    }

    SDCardMSC sdcardmsc;
}
