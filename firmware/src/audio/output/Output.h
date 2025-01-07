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
        void MakeSampleStereo16(int16_t sample[2]) 
        {
            // Mono to "stereo" conversion
            if (channels == 1)
                sample[RIGHTCHANNEL] = sample[LEFTCHANNEL];
            if (bps == 8) 
            {
                // Upsample from unsigned 8 bits to signed 16 bits
                sample[LEFTCHANNEL] = (((int16_t)(sample[LEFTCHANNEL]&0xff)) - 128) << 8;
                sample[RIGHTCHANNEL] = (((int16_t)(sample[RIGHTCHANNEL]&0xff)) - 128) << 8;
            }
        };

        inline int16_t Amplify(int16_t s) 
        {
            int32_t v = (s * gainF2P6) >> 6;
            if (v < -32767) return -32767;
            else if (v > 32767) return 32767;
            else return (int16_t)(v & 0xffff);
        }

    protected:
        uint16_t hertz;
        uint8_t bps;
        uint8_t channels;
        uint8_t gainF2P6; // Fixed point 2.6
  
    };
}
