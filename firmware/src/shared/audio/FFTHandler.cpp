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

    void FFTHandler::init(audio_tools::AudioFFTBase &fft)
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
                m_bins[i].idx = parabolicMap(0, last, minBinIndex, maxBinIndex, i);
                m_bins[i].frq = fft.frequency(m_bins[i].idx);
                m_bins[i].mag = 0.f;

                log_i("%d: %d - %d", i, m_bins[i].idx, m_bins[i].frq);
            }
        }
    }

    void FFTHandler::update(audio_tools::AudioFFTBase &fft)
    {
        for (uint8_t b = 0, last = m_numBins - 1; b <= last; ++b)
        {
            if (b == 0)
            {
                static float mag = 0;
                if (fft.magnitude(m_bins[b].idx) > mag)
                {
                    mag = fft.magnitude(m_bins[b].idx);
                    log_i("%f", mag);
                }
            }

            float magnitude = fft.magnitude(m_bins[b].idx);

            uint8_t binsBetween;
            if (b > 0 && b < last)
            {
                binsBetween = m_bins[b].idx - m_bins[b - 1].idx;
                for (uint8_t i = 0; i < binsBetween; ++i)
                {
                    auto scale = float(i) / float(binsBetween);
                    auto index = m_bins[b].idx - binsBetween + i;
                    magnitude += scale * fft.magnitude(index);
                }

                binsBetween = m_bins[b + 1].idx - m_bins[b].idx;
                for (uint8_t i = 0; i < binsBetween; ++i)
                {
                    auto scale = float(i) / float(binsBetween);
                    auto index = m_bins[b].idx + binsBetween - i;
                    magnitude += scale * fft.magnitude(index);
                }
            }

            //

            m_bins[b].mag = constrain(0.000001f * magnitude, 0.f, 1.f);
        }
    }
}
