#pragma once

#include <stdint.h>

#define POWER_SOURCE_VOLTAGE_PIN GPIO_NUM_3
#define POWER_SOURCE_READ_PERIOD 1000

namespace driver
{
    class PowerSource
    {
        enum
        {
            BatMinMilliVolts = 3450, // 3.45V
            BatMaxMilliVolts = 4250, // 4.25V
            UsbMinMilliVolts = 4750, // 4.75V
            UsbMaxMilliVolts = 5250  // 5.25V 
        };

        using Timestamp = unsigned long;
        using MilliVolt = uint32_t;

    public:
        enum class Type { Unknown, Battery, USB };

        void  begin();
        Type  getType();
        float getInputVoltage();
        float getBatteryLevel();
        int   getPatteryLevelPercents();

    private:
        MilliVolt getInputMilliVoltsCached();
        MilliVolt getInputMilliVoltsRaw();

    private:
        Timestamp m_timestamp;
        MilliVolt m_measurement;
    };
    
    extern PowerSource powerSource;
}
