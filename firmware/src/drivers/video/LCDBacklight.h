#pragma once

#include <stdint.h>

namespace driver
{
    class LCDBacklight
    {
    public:
        using Brightness = uint8_t;
        constexpr static Brightness RANGE = 200;

        void begin(Brightness brightness);
        void setBrightness(Brightness brightness);
        void setBrightness(float brightness);
    };

    extern LCDBacklight lcdBacklight;
}
