#pragma once

#include <TFT_eSPI.h>
#include "shared/audio/FFTHandler.h"

namespace image
{
    class Spectrum : public audio::FFTHandler
    {
    public:
        using Frequency = audio::FFTHandler::Frequency;
        using Magnitude = audio::FFTHandler::Magnitude;

        Spectrum(uint8_t numBins = 32, Frequency minFreq = 80, Frequency maxFreq = 16000);

        void renderOn(TFT_eSprite& sprite, uint8_t gap);

    private:
        uint32_t m_bgColor;
        uint32_t m_fgColor;
    };
}