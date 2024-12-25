#include "SDCardMSC.h"
#include "SDCard.h"
#include "Flash.h"
#include <USB.h>
#include <esp_task_wdt.h>



namespace driver
{
    static int32_t onWriteSDCard(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
    {
        sdcard.writeSectors(buffer, lba, bufsize / sdcard.getSectorSize());
        return bufsize;
    }

    static int32_t onReadSDCard(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
    {
        sdcard.readSectors((uint8_t *)buffer, lba, bufsize / sdcard.getSectorSize());
        return bufsize;
    }

    static int32_t onWriteFlash(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
    {
        flash.writeSectors(buffer, lba, bufsize / flash.getSectorSize());
        return bufsize;
    }

    static int32_t onReadFlash(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
    {
        flash.readSectors((uint8_t *)buffer, lba, bufsize / flash.getSectorSize());
        return bufsize;
    }

    static bool onStartStop(uint8_t power_condition, bool start, bool load_eject)
    {
        if (load_eject && !start)
        {
            sdcardmsc.stopMSC();
        }
        return true;
    }

    void SDCardMSC::startMSC(bool useExtStorage)
    {
        if (flash.isMounted() && !_run)
        {
            _run = USB.begin();
            _msc.vendorID("ESP32-S3");
            _msc.productID("SmallTV");
            _msc.productRevision("1.0");
            _msc.mediaPresent(true);
            _msc.onStartStop(onStartStop);

            if (useExtStorage && sdcard.isMounted())
            {
                _msc.onRead(onReadSDCard);
                _msc.onWrite(onWriteSDCard);
                _run &= _msc.begin(sdcard.getSectorCount(), sdcard.getSectorSize());
            }
            else
            {
                _msc.onRead(onReadFlash);
                _msc.onWrite(onWriteFlash);
                _run &= _msc.begin(flash.getSectorCount(), flash.getSectorSize());
            }
        }
    }

    void SDCardMSC::stopMSC()
    {
        if (_run)
        {
            _run = false;
            _msc.end();
        }
    }

    bool SDCardMSC::isRunning() const
    {
        return _run;
    }

    SDCardMSC sdcardmsc;
}
