#include "Spectrum.h"

namespace image
{
    Spectrum::Spectrum(uint8_t numBins, Frequency minFreq, Frequency maxFreq)
        : audio::FFTHandler(numBins, minFreq, maxFreq)
        , m_bins(std::make_unique<uint8_t[]>(numBins))
    {
    }

    void Spectrum::renderOn(TFT_eSprite &sprite)
    {
        task::LockGuard lock(m_mutex);
        sprite.fillSprite(TFT_BLACK);
        
        // TODO
    }

    void Spectrum::onUpdate()
    {
        task::LockGuard lock(m_mutex);
        for (uint8_t i = 0, c = getBinCount(); i < c; ++i)
        {
            // TODO
            m_bins[i] = getMagnitude(i);
        }
    }
}
