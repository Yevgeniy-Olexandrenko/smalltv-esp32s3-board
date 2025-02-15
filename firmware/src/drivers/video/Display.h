#pragma once

#include <TFT_eSPI.h>
#include <freertos/task.h>

namespace driver
{
    class Display : public TFT_eSPI
    {
    public:
        void begin(float brightness);
        void setBrightness(float brightness);
        void fadeIn();
        void fadeOut();

    private:
        bool needsAdjustment() const;
        void setTagretAndWait(int16_t value);
        void brightnessTask();

    private:
        int16_t m_brGlobal = 0;
        int16_t m_brTarget = 0;
        int16_t m_brFade = 0;
    };

    extern Display display;
}
