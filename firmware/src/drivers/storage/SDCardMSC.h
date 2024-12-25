#pragma once

#include <USBMSC.h>

namespace driver
{
    class SDCardMSC
    {
    public:
        void begin();
        void end();

        bool isRunning() const;

    private:
        USBMSC _msc;
        bool _run = false;
    };

    extern SDCardMSC sdcardmsc;
}
