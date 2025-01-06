#pragma once

#include <stdint.h>

namespace audio
{
    class Output
    {
    public:
        enum SampleIndex { LEFTCHANNEL = 0, RIGHTCHANNEL = 1 };

        Output() { };
        virtual ~Output() {};

        virtual bool SetRate(int hz) { hertz = hz; return true; }
        virtual bool SetBitsPerSample(int bits) { bps = bits; return true; }
        virtual bool SetChannels(int chan) { channels = chan; return true; }
        virtual bool SetGain(float f)
        {
            if (f > 4.0) f = 4.0;
            if (f < 0.0) f = 0.0;
            gainF2P6 = (uint8_t)(f * (1 << 6));
            return true;
        }
        virtual bool begin() { return false; };
        virtual bool ConsumeSample(int16_t sample[2]) { return false; }
        virtual uint16_t ConsumeSamples(int16_t* samples, uint16_t count)
        {
            for (uint16_t i = 0; i < count; i++)
            {
                if (!ConsumeSample(samples)) return i;
                samples += 2;
            }
            return count;
        }
        virtual bool stop() { return false; }
        virtual void flush() {}
        virtual bool loop() { return true; }

    protected:
        uint16_t hertz;
        uint8_t bps;
        uint8_t channels;
        uint8_t gainF2P6; // Fixed point 2.6
  
    };
}
