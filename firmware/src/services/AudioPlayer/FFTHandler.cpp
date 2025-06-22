#ifndef NO_AUDIO

#include "FFTHandler.h"

namespace service::details
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
        , m_numBins(numBins + 2)
        , m_bins(std::make_unique<Bin[]>(m_numBins))
    {}

    void FFTHandler::init(audio_tools::AudioFFTBase &fft)
    {
        auto topFreq = fft.audioInfoOut().sample_rate / 2;
        auto minFreq = constrain(m_minFreq, 20, topFreq);
        auto maxFreq = constrain(m_maxFreq, 20, topFreq);

        log_i("topFreq: %d", topFreq);
        log_i("minFreq: %d", minFreq);
        log_i("maxFreq: %d", maxFreq);

        if (minFreq < maxFreq)
        {
            auto minBinIndex = fft.frequencyToBin(minFreq);
            auto maxBinIndex = fft.frequencyToBin(maxFreq);

            log_i("minBinIndex: %d", minBinIndex);
            log_i("maxBinIndex: %d", maxBinIndex);

            for (uint8_t i = 0, last = m_numBins - 1; i <= last; ++i)
            {
                m_bins[i].idx = parabolicMap(0, last, minBinIndex, maxBinIndex, i);
                m_bins[i].frq = fft.frequency(m_bins[i].idx);
                m_bins[i].mag = 0.f;

                log_i("Bin %d: %d - %d", i, m_bins[i].idx, m_bins[i].frq);
            }
        }

        m_maxUpdate = millis();
        m_maxOldMag = 1000000;
        m_maxNewMag = 0;
    }

    void FFTHandler::update(audio_tools::AudioFFTBase &fft)
    {
        const float normalization = 1.f / m_maxOldMag;
        for (uint8_t binsBetween, b = 0, last = m_numBins - 1; b <= last; ++b)
        {
            float magnitude = fft.magnitude(m_bins[b].idx);
            if (magnitude > m_maxNewMag) m_maxNewMag = magnitude;
            
            if (b > 0)
            {
                binsBetween = m_bins[b].idx - m_bins[b - 1].idx;
                for (uint8_t i = 0; i < binsBetween; ++i)
                {
                    auto scale = float(i) / float(binsBetween);
                    auto index = m_bins[b].idx - binsBetween + i;
                    magnitude += scale * fft.magnitude(index);
                }
            }
            if (b < last)
            {
                binsBetween = m_bins[b + 1].idx - m_bins[b].idx;
                for (uint8_t i = 0; i < binsBetween; ++i)
                {
                    auto scale = float(i) / float(binsBetween);
                    auto index = m_bins[b].idx + binsBetween - i;
                    magnitude += scale * fft.magnitude(index);
                }
            }

            magnitude = constrain(magnitude * normalization, 0.f, 1.f);
            m_bins[b].mag = magnitude;
        }

        // if (millis() - m_maxUpdate >= MAX_UPDATE_TIME)
        // {
        //     m_maxUpdate += MAX_UPDATE_TIME;
        //     m_maxOldMag *= (1.f - MAX_SMOOTH_STEP);
        //     m_maxOldMag += m_maxNewMag * MAX_SMOOTH_STEP;
        //     m_maxNewMag = 0;
        // }
    }

    uint8_t FFTHandler::getBinCount() const
    {
        return (m_numBins - 2);
    }

    FFTHandler::Frequency FFTHandler::getFrequency(uint8_t bin) const
    { 
        return (bin < getBinCount() ? m_bins[bin + 1].frq : 0); 
    }

    FFTHandler::Magnitude FFTHandler::getMagnitude(uint8_t bin) const 
    { 
        return (bin < getBinCount() ? m_bins[bin + 1].mag : 0); 
    }
}
#endif
