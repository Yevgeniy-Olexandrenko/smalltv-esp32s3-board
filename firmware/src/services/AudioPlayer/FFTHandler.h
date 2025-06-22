#pragma once
#ifndef NO_AUDIO

#include <memory>
#include <AudioTools/AudioLibs/AudioFFT.h>

#define MAX_UPDATE_TIME 100
#define MAX_SMOOTH_STEP 0.1

namespace service::details
{
    class FFTHandler
    {
    public:
        using Frequency = uint16_t;
        using Magnitude = float;

        FFTHandler(uint8_t numBins = 32, Frequency minFreq = 80, Frequency maxFreq = 16000);

        virtual void init(audio_tools::AudioFFTBase& fft);
        virtual void update(audio_tools::AudioFFTBase& fft);

        uint8_t getBinCount() const;
        Frequency getFrequency(uint8_t bin) const;
        Magnitude getMagnitude(uint8_t bin) const;

    private:
        const uint8_t m_numBins;
        const Frequency m_minFreq;
        const Frequency m_maxFreq;

        struct Bin { int idx; Frequency frq; float mag; };
        std::unique_ptr<Bin[]> m_bins;

        unsigned long m_maxUpdate;
        float m_maxNewMag;
        float m_maxOldMag;
    };
}
#endif
