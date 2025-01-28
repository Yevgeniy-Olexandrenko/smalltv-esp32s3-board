#include "PowerSource.h"

#define UPDATE_PERIOD 5000

namespace driver
{
    void PowerSource::begin()
    {
        _timestamp = millis();
        _milliVolts = getInputMilliVoltsRaw();
    }

    PowerSource::Type PowerSource::getType()
    {
        MilliVolt milliVolts = getInputMilliVoltsCached();
        if (milliVolts >= BatMinMilliVolts && milliVolts <= BatMaxMilliVolts) return Type::Battery;
        if (milliVolts >= UsbMinMilliVolts && milliVolts <= UsbMaxMilliVolts) return Type::USB;
        return Type::Unknown;
    }

    bool PowerSource::isOutOfRange()
    {
        MilliVolt milliVolts = getInputMilliVoltsCached();
        return (milliVolts < BatMinMilliVolts || milliVolts > UsbMaxMilliVolts);
    }

    float PowerSource::getInputVoltage()
    {
        return (0.001f * getInputMilliVoltsCached());
    }

    float PowerSource::getBatteryLevel()
    {
        if (getType() != Type::Battery) return -1;
        return (0.01f * getBatteryLevelPercents());
    }

    int PowerSource::getBatteryLevelPercents()
    {
        if (getType() == Type::Battery)
        {
            float voltage = getInputVoltage();
            if (voltage > 4.19f) return 100;
            if (voltage < 3.50f) return 0;
            float percent = 
                  2808.3808f * pow(voltage, 4) - 
                 43560.9157f * pow(voltage, 3) + 
                252848.5888f * pow(voltage, 2) - 
                650767.4615f * voltage + 
                626532.5703f;
            return int(percent + 0.5f);
        }
        return -1;
    }

    PowerSource::MilliVolt PowerSource::getInputMilliVoltsCached()
    {
    #ifndef NO_VINSENSE
        if (millis() - _timestamp >= UPDATE_PERIOD)
        {
            _timestamp = millis();
            _milliVolts += getInputMilliVoltsRaw();
            _milliVolts /= 2;
        }
        return _milliVolts;
    #else
        return 0;
    #endif
    }

    PowerSource::MilliVolt PowerSource::getInputMilliVoltsRaw()
    {
    #ifndef NO_VINSENSE
        pinMode(PIN_VIN_SEN, INPUT);
        analogSetPinAttenuation(PIN_VIN_SEN, ADC_11db);

        // calculation of the coefficients of 
        // a line based on two existing points
        constexpr float k = float(VIN_SEN_VOL2 - VIN_SEN_VOL1) / float(VIN_SEN_ADC2 - VIN_SEN_ADC1);
        constexpr float b = float(VIN_SEN_VOL1) - float(k * VIN_SEN_ADC1); 

        // calculation of voltage based on 
        // a calibrated dependence line
        auto x = analogReadRaw(PIN_VIN_SEN);
        auto y = MilliVolt(k * x + b);
        log_i("%d - %d", x, y);
        return y;
    #else
        return 0;
    #endif
    }

    PowerSource powerSource;
}
