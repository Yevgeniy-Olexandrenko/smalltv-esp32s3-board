#pragma once
#ifndef NO_AUDIO

#include <TFT_eSPI.h>
#include "core/Core.h"
#include "services/AudioPlayer/FFTHandler.h"

namespace image
{
    class Spectrum : public service::details::FFTHandler
    {
    public:
        using Frequency = service::details::FFTHandler::Frequency;
        using Magnitude = service::details::FFTHandler::Magnitude;

        Spectrum(uint8_t numBins = 32, Frequency minFreq = 80, Frequency maxFreq = 16000);

        void init(audio_tools::AudioFFTBase& fft) override;
        void update(audio_tools::AudioFFTBase& fft) override;

        void renderOn(TFT_eSprite& sprite, uint8_t gap, float smooth);

    private:
        uint32_t m_bgColor;
        uint32_t m_fgColor;
        std::unique_ptr<uint8_t[]> m_barH;

        core::Mutex m_mutex;
        bool m_isUpdated;
    };
}
#endif
