#pragma once

#include <TFT_eSPI.h>
#include <freertos/task.h>

namespace driver
{
    class Display : public TFT_eSPI
    {
    public:
        Display();

        void begin(float brightness);
        void setBrightness(float brightness);
        float getBrightness() const;

    private:
        void brightnessUpdateTask();

    private:
        TaskHandle_t m_handle;
        int16_t m_pwmCur;
        int16_t m_pwmTar;
    };

    extern Display display;
}
