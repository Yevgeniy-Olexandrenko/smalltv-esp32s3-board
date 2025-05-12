#include "Spectrum.h"

namespace image
{
    Spectrum::Spectrum(uint8_t numBins, Frequency minFreq, Frequency maxFreq)
        : audio::FFTHandler(numBins, minFreq, maxFreq)
        , m_fgColor(TFT_WHITE)
        , m_bgColor(TFT_BLACK)
    {
    }

    void Spectrum::renderOn(TFT_eSprite& sprite, uint8_t gap)
    {
        auto numBars = getBinCount();
        auto height = sprite.height();
        auto width = sprite.width();
        auto barWidth = float(width) / numBars - gap;

        sprite.fillSprite(m_bgColor);
        for (int i = 0; i < numBars; i++) 
        {
            int barHeight = getMagnitude(i) * height;
            
            if (barHeight > height) barHeight = height;
            if (barHeight < 0) barHeight = 0;

            auto x = int(i * (barWidth + gap));
            auto y = height - barHeight;

            sprite.fillRect(x, y, barWidth, barHeight, m_fgColor);
        }
    }
}
