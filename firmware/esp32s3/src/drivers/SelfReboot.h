#pragma once

namespace driver
{
    class SelfReboot
    {
    public:
        void begin();
        void reboot();
    };

    extern SelfReboot selfReboot;
}
