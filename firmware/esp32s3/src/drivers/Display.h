#pragma once
#ifndef NO_VIDEO

#include <TFT_eSPI.h>
#include "core/Core.h"

namespace driver
{
    class Display 
        : public TFT_eSPI
        , public core::Task<1024, core::TaskCpu::System, core::TaskPrio::Background>
    {
    public:
        void begin(float brightness);
        void setBrightness(float brightness);
        void fadeIn();
        void fadeOut();

    private:
        bool needsAdjustment() const;
        void setTagretAndWait(int16_t value);
        void task() override;

    private:
        int16_t m_brGlobal;
        int16_t m_brTarget;
        int16_t m_brFade;
    };

    extern Display display;
}
#endif
