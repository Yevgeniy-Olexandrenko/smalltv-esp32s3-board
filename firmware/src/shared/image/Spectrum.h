#pragma once

#include <TFT_eSPI.h>
#include "shared/audio/FFTHandler.h"
#include "shared/tasks/Mutex.h"

namespace image
{
    class Spectrum : public audio::FFTHandler
    {
    public:
        using Frequency = audio::FFTHandler::Frequency;
        using Magnitude = audio::FFTHandler::Magnitude;

        Spectrum(uint8_t numBins = 32, Frequency minFreq = 80, Frequency maxFreq = 16000);

        void renderOn(TFT_eSprite& sprite);

    protected:
        void onUpdate() override; 

    private:
        std::unique_ptr<uint8_t[]> m_bins;
        task::Mutex m_mutex;
    };
}