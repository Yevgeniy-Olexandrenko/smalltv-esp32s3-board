#pragma once

#include <USBMSC.h>

namespace driver
{
    class SDCardMSC
    {
    public:
        void startMSC(bool useExtStorage);
        void stopMSC();

        bool isRunning() const;

    private:
        USBMSC _msc;
        bool _run = false;
    };

    extern SDCardMSC sdcardmsc;
}
