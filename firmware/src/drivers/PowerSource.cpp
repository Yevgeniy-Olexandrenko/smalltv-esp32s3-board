#include "PowerSource.h"
#include <Arduino.h>

namespace driver
{
    void PowerSourceClass::beging()
    {
        m_measurement = getInputMilliVoltsRaw();
        m_timestamp = millis();
    }

    PowerSourceClass::Type PowerSourceClass::getType()
    {
        MilliVolt milliVolts = getInputMilliVoltsCached();
        if (milliVolts >= BatMinMilliVolts && milliVolts <= BatMaxMilliVolts) return Type::Battery;
        if (milliVolts >= UsbMinMilliVolts && milliVolts <= UsbMaxMilliVolts) return Type::USB;
        return Type::Unknown;
    }

    float PowerSourceClass::getInputVoltage()
    {
        return (0.001f * getInputMilliVoltsCached());
    }

    float PowerSourceClass::getBatteryLevel()
    {
        if (getType() != Type::Battery) return -1;
        return (0.01f * getPatteryLevelPercents());
    }

    int PowerSourceClass::getPatteryLevelPercents()
    {
        if (getType() == Type::Battery)
        {
            float voltage = getInputVoltage();
            if (voltage > 4.19f) return 100;
            if (voltage < 3.50f) return 0;

            float percent = 2808.3808f * pow(voltage, 4) - 43560.9157f * pow(voltage, 3) + 
                252848.5888f * pow(voltage, 2) - 650767.4615f * voltage + 626532.5703f;
            return int(percent + 0.5f);
        }
        return -1;
    }

    PowerSourceClass::MilliVolt PowerSourceClass::getInputMilliVoltsCached()
    {
        if ((millis() - m_timestamp) >= POWER_SOURCE_READ_PERIOD)
        {
            m_measurement = getInputMilliVoltsRaw();
            m_timestamp = millis();
        }
        return m_measurement;
    }

    PowerSourceClass::MilliVolt PowerSourceClass::getInputMilliVoltsRaw()
    {
        analogSetPinAttenuation(POWER_SOURCE_VOLTAGE_PIN, ADC_11db);
        return (2 * analogReadMilliVolts(POWER_SOURCE_VOLTAGE_PIN));
    }

    PowerSourceClass PowerSource;
}
