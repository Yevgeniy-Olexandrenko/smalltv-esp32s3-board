#ifndef NO_VIDEO

#include "Display.h"
#include "hardware/board.h"

#define LCD_BL_PWM_CHANNEL   0
#define LCD_BL_PWM_FREQENCY  5000
#define LCD_BL_PWM_MIN_DELTA 2

namespace driver
{
    void Display::begin(float brightness)
    {
        if (TFT_eSPI::_booted)
        {
            TFT_eSPI::init();
            TFT_eSPI::setRotation(0);
            TFT_eSPI::fillScreen(TFT_BLACK);

            setBrightness(brightness);
            Task::start("lcd_brightness");
        }
    }

    void Display::setBrightness(float brightness)
    {
        brightness = constrain(brightness, 0.f, 1.f);
        m_brGlobal = int16_t(103 + brightness * 920);
        m_brTarget = m_brGlobal;
    }

    void Display::fadeIn()
    {
        setTagretAndWait(m_brGlobal);
    }

    void Display::fadeOut()
    {
        setTagretAndWait(0);
    }

    bool Display::needsAdjustment() const
    {
        return (abs(m_brTarget - m_brFade) >= LCD_BL_PWM_MIN_DELTA);
    }

    void Display::setTagretAndWait(int16_t value)
    {
        m_brTarget = value;
        while(needsAdjustment()) taskYIELD();
    }

    void Display::task()
    {
        ledcSetup(LCD_BL_PWM_CHANNEL, LCD_BL_PWM_FREQENCY, 10);
        ledcAttachPin(PIN_LCD_BL, LCD_BL_PWM_CHANNEL);

        for (m_brFade = 0;;)
        {
            if (needsAdjustment())
            {
                if (m_brTarget > m_brFade)
                    m_brFade += LCD_BL_PWM_MIN_DELTA;
                else
                    m_brFade -= LCD_BL_PWM_MIN_DELTA;
                m_brFade = constrain(m_brFade, 0, 1023);

                auto pwm = uint32_t((uint32_t(m_brFade + 1) * (m_brFade + 1) * m_brFade) >> 20);
                #ifdef LCD_BL_INV
                pwm = (1023 - pwm);
                #endif

                ledcWrite(LCD_BL_PWM_CHANNEL, pwm);
            }
            sleep(1);
        }
    }

    Display display;
}
#endif
