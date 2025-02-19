#pragma once

#include "onboard/PowerSource.h"
#include "onboard/StatusLED.h"
#include "onboard/SelfReboot.h"
#include "storage/Flash.h"
#include "storage/SDCard.h"
#include "storage/Storage.h"
#include "video/Display.h"
#include "board.h"

namespace drivers
{
    void begin();
}

namespace hardware
{
    inline const bool hasVINSense()
    {
    #ifdef NO_VINSENSE
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasSelfReboot()
    {
    #ifdef NO_SELFRES
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasPushButton()
    {
    #ifdef NO_BUTTON
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasStatusLED()
    {
    #ifdef NO_LED
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasSDCard()
    {
    #ifdef NO_SDCARD
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasAudio()
    {
    #ifdef NO_SOUND
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasMicrophone()
    {
    #ifdef NO_MICROPHONE
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasTouchButton()
    {
    #ifdef NO_TOUCH0
        return false;
    #else
        return true;
    #endif
    }

    inline const bool hasPSRAM()
    {
    #ifdef BOARD_HAS_PSRAM
        return psramFound();
    #else
        return false;
    #endif
    }

    inline const bool hasUsbMSC()
    {
    #if ARDUINO_USB_MODE == 0
        return false;
    #else
        return true;
    #endif
    }
}
