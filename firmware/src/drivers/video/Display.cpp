#include <freertos/FreeRTOS.h>
#include "Display.h"
#include "board.h"

#define LCD_BL_PWM_CHANNEL  0
#define LCD_BL_PWM_FREQENCY 5000
#define LCD_BL_PWM_DELTA    2

namespace driver
{
    Display::Display()
        : TFT_eSPI()
        , m_handle(nullptr)
        , m_pwmCur(0)
        , m_pwmTar(0)
    {}

    void Display::begin(float brightness)
    {
        TFT_eSPI::init();
        TFT_eSPI::setRotation(0);
        TFT_eSPI::fillScreen(TFT_BLACK);
        setBrightness(brightness);
    }

    void Display::setBrightness(float brightness)
    {
        brightness = constrain(brightness, 0.f, 1.f);
        m_pwmTar = uint16_t(brightness * 1023);

        if (!m_handle)
        {
            xTaskCreatePinnedToCore(
                [](void* data) 
                {
                    Display* display = static_cast<Display*>(data);
                    display->brightnessUpdateTask();
                },
                "brightness",
                2048,
                this,
                1,
                &m_handle,
                1
            );
        }
    }

    float Display::getBrightness() const
    {
        return (float(m_pwmTar) / 1023);
    }

    void Display::brightnessUpdateTask()
    {
        ledcSetup(LCD_BL_PWM_CHANNEL, LCD_BL_PWM_FREQENCY, 10);
        ledcAttachPin(PIN_LCD_BL, LCD_BL_PWM_CHANNEL);

        while(true)
        {
            auto pwmTar = m_pwmTar;
            if (abs(pwmTar - m_pwmCur) >= LCD_BL_PWM_DELTA)
            {
                m_pwmCur += (pwmTar > m_pwmCur ? +(LCD_BL_PWM_DELTA) : -(LCD_BL_PWM_DELTA));
                m_pwmCur = constrain(m_pwmCur, 0, 1023);
                auto pwmCRT = uint32_t((uint32_t(m_pwmCur + 1) * (m_pwmCur + 1) * m_pwmCur) >> 20);
                if (!pwmCRT) pwmCRT = 1;
            #ifdef LCD_BL_INV
                pwmCRT = (1023 - pwmCRT);
            #endif
                ledcWrite(LCD_BL_PWM_CHANNEL, pwmCRT);
                vTaskDelay(pdMS_TO_TICKS(1));
            } else taskYIELD();
        }
    }

    Display display;
}
