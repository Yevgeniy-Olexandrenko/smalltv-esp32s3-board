#pragma once

#include <memory>
#include <AudioTools/AudioLibs/AudioFFT.h>

namespace audio
{
    class FFTHandler
    {
    public:
        using Frequency = uint16_t;
        using Magnitude = float;

        FFTHandler(uint8_t numBins = 32, Frequency minFreq = 80, Frequency maxFreq = 16000);

        void init(audio_tools::AudioFFTBase& fft);
        void update(audio_tools::AudioFFTBase& fft);

        uint8_t getBinCount() const { return m_numBins; }
        Frequency getFrequency(uint8_t bin) const { return (bin < m_numBins ? m_bins[bin].f : 0); }
        Magnitude getMagnitude(uint8_t bin) const { return (bin < m_numBins ? m_bins[bin].f : 0); }

    protected:
        virtual void onUpdate() {}

    private:
        const uint8_t m_numBins;
        const Frequency m_minFreq;
        const Frequency m_maxFreq;

        struct Bin { int i; Frequency f; float m; };
        std::unique_ptr<Bin[]> m_bins;
    };
}
