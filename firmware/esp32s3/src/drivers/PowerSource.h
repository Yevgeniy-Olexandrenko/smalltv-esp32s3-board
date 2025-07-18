#pragma once
#ifndef NO_VINSENSE

#include "core/Core.h"

namespace driver
{
    class PowerSource
    {
        constexpr static auto UPDATE_PERIOD_SEC = 5;

        enum
        {
            BatMinMilliVolts = 3450, // 3.45V
            BatMaxMilliVolts = 4250, // 4.25V
            UsbMinMilliVolts = 4750, // 4.75V
            UsbMaxMilliVolts = 5250  // 5.25V 
        };

        using MilliVolt = uint16_t;

    public:
        enum class Type { Unknown, Battery, USB };

        void  begin();
        Type  getType();
        bool  isOutOfRange();
        float getInputVoltage();
        float getBatteryLevel();
        int   getBatteryLevelPercents();

    private:
        MilliVolt getInputMilliVoltsCached();
        MilliVolt getInputMilliVoltsRaw();

    private:
        core::TimerSec<UPDATE_PERIOD_SEC> m_timer;
        MilliVolt m_milliVolts;
    };
    
    extern PowerSource powerSource;
}
#endif
