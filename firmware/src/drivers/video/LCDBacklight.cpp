#include <Arduino.h>
#include "LCDBacklight.h"
#include "board.h"

#define LCD_BL_PWM_CHANNEL    0
#define LCD_BL_PWM_FREQENCY   5000
#define LCD_BL_PWM_RESOLUTION 10

namespace driver
{
    void LCDBacklight::begin(Brightness brightness)
    {
        ledcSetup(LCD_BL_PWM_CHANNEL, LCD_BL_PWM_FREQENCY, LCD_BL_PWM_RESOLUTION);
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
    #if LCD_BL_PWM_RESOLUTION == 8
        auto pwmRaw = uint16_t(brightness * 255);
        auto pwmCRT = uint32_t((uint32_t(pwmRaw + 1) * (pwmRaw + 1) * pwmRaw) >> 16);
    #elif LCD_BL_PWM_RESOLUTION == 10
        auto pwmRaw = uint16_t(brightness * 1023);
        auto pwmCRT = uint32_t((uint32_t(pwmRaw + 1) * (pwmRaw + 1) * pwmRaw) >> 20);
    #endif
        ledcWrite(LCD_BL_PWM_CHANNEL, pwmCRT ? pwmCRT : 1);
    }

    LCDBacklight lcdBacklight;
}