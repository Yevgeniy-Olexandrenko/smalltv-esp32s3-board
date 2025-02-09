#include <Arduino.h>
#include "LCDBacklight.h"
#include "board.h"

#define LCD_BL_PWM_CHANNEL    0
#define LCD_BL_PWM_FREQENCY   5000

namespace driver
{
    void LCDBacklight::begin(Brightness brightness)
    {
        ledcSetup(LCD_BL_PWM_CHANNEL, LCD_BL_PWM_FREQENCY, 10);
        ledcAttachPin(PIN_LCD_BL, LCD_BL_PWM_CHANNEL);
        setBrightness(brightness);
    }

    void LCDBacklight::setBrightness(Brightness brightness)
    {
        if (brightness > RANGE) brightness = RANGE;
        setBrightness(float(brightness) / RANGE);
    }

    void LCDBacklight::setBrightness(float brightness)
    {
        brightness = constrain(brightness, 0.f, 1.f);
        auto pwmRaw = uint16_t(brightness * 1023);
        auto pwmCRT = uint32_t((uint32_t(pwmRaw + 1) * (pwmRaw + 1) * pwmRaw) >> 20);
        if (!pwmCRT) pwmCRT = 1;
    #ifdef LCD_BL_INV
        pwmCRT = (1023 - pwmCRT);
    #endif
        ledcWrite(LCD_BL_PWM_CHANNEL, pwmCRT);
    }

    LCDBacklight lcdBacklight;
}