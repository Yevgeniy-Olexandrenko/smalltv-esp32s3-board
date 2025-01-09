#pragma once

#include "Decode.h"
#include "libhelix-mp3/mp3dec.h"

namespace audio
{
    class DecodeMP3 : public Decode
    {
    public:
        DecodeMP3();
        ~DecodeMP3() override;

        bool begin(Source* source, Output* output) override;
        bool loop() override;
        bool stop() override;

    protected:
        // Helix MP3 decoder
        HMP3Decoder hMP3Decoder;

        // Input buffering
        uint8_t buff[1600]; // File buffer required to store at least a whole compressed frame
        int16_t buffValid;
        int16_t lastFrameEnd;
        bool FillBufferWithValidFrame(); // Read until we get a valid syncword and min(feof, 2048) butes in the buffer

        // Output buffering
        int16_t outSample[1152 * 2]; // Interleaved L/R
        int16_t validSamples;
        int16_t curSample;

        // Each frame may change this if they're very strange, I guess
        unsigned int lastRate;
        int lastChannels;
    };
}
