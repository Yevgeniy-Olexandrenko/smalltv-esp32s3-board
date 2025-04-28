#include "FFTHandler.h"

namespace audio
{
    float parabolicMap(float x0, float x1, float y0, float y1, float x)
    {
        if (x1 == x0) return y0;
        float t = ((x - x0) / (x1 - x0));
        return (y0 + (y1 - y0) * t * t);
    };

    FFTHandler::FFTHandler(uint8_t numBins, Frequency minFreq, Frequency maxFreq)
        : m_minFreq(minFreq)
        , m_maxFreq(maxFreq)
        , m_numBins(numBins)
        , m_bins(std::make_unique<Bin[]>(numBins))
    {}

    void FFTHandler::begin(audio_tools::AudioFFTBase &fft)
    {
        auto topFreq = fft.audioInfoOut().sample_rate / 2;
        auto minFreq = constrain(m_minFreq, 20, topFreq);
        auto maxFreq = constrain(m_maxFreq, 20, topFreq);

        if (minFreq < maxFreq)
        {
            auto minBinIndex = fft.frequencyToBin(minFreq);
            auto maxBinIndex = fft.frequencyToBin(maxFreq);

            for (uint8_t i = 0, last = m_numBins - 1; i <= last; ++i)
            {
                m_bins[i].i = parabolicMap(0, last, minBinIndex, maxBinIndex, i);
                m_bins[i].f = fft.frequency(m_bins[i].i);
                m_bins[i].m = 0.f;
            }
        }
    }

    void FFTHandler::update(audio_tools::AudioFFTBase &fft)
    {
        for (uint8_t b = 0, last = m_numBins - 1; b <= last; ++b)
        {
            float magnitude = fft.magnitude(m_bins[b].i);

            uint8_t binsBetween;
            if (b > 0 && b < last)
            {
                binsBetween = m_bins[b].i - m_bins[b - 1].i;
                for (uint8_t i = 0; i < binsBetween; ++i)
                {
                    auto scale = float(i) / float(binsBetween);
                    auto index = m_bins[b].i - binsBetween + i;
                    magnitude += scale * fft.magnitude(index);
                }

                binsBetween = m_bins[b + 1].i - m_bins[b].i;
                for (uint8_t i = 0; i < binsBetween; ++i)
                {
                    auto scale = float(i) / float(binsBetween);
                    auto index = m_bins[b].i + binsBetween - i;
                    magnitude += scale * fft.magnitude(index);
                }
            }


            m_bins[b].m = 0;
        }
        onUpdate();
    }
}
