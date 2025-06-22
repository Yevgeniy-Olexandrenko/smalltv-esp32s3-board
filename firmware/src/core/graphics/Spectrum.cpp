#ifndef NO_AUDIO

#include "Spectrum.h"

namespace image
{
    Spectrum::Spectrum(uint8_t numBins, Frequency minFreq, Frequency maxFreq)
        : service::details::FFTHandler(numBins, minFreq, maxFreq)
        , m_fgColor(TFT_WHITE)
        , m_bgColor(TFT_NAVY)
        , m_barH(std::make_unique<uint8_t[]>(numBins))
    {
    }

    void Spectrum::init(audio_tools::AudioFFTBase& fft)
    {
        FFTHandler::init(fft);
        m_isUpdated = true;
    }

    void Spectrum::update(audio_tools::AudioFFTBase& fft)
    {
        task::LockGuard lock(m_mutex);
        FFTHandler::update(fft);
        m_isUpdated = true;
    }

    void Spectrum::renderOn(TFT_eSprite& sprite, uint8_t gap, float smooth)
    {
        task::LockGuard lock(m_mutex);
        if (m_isUpdated)
        {
            m_isUpdated = false;

            auto bars = getBinCount();
            auto sprH = sprite.height();
            auto sprW = sprite.width();
            auto barW = float(sprW) / bars - gap;

            sprite.fillSprite(m_bgColor);
            for (int i = 0; i < bars; i++) 
            {
                uint8_t barH = getMagnitude(i) * sprH * smooth + m_barH[i] * (1.f - smooth);
                m_barH[i] = barH;
                
                auto x = int32_t(i * (barW + gap));
                auto y = int32_t(sprH - barH);

                sprite.fillRect(x, y, barW, barH, m_fgColor);
            }
        }
    }
}
#endif
