#pragma once
#ifndef NO_VINSENSE

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
        Timestamp m_timestamp;
        MilliVolt m_milliVolts;
    };
    
    extern PowerSource powerSource;
}
#endif
