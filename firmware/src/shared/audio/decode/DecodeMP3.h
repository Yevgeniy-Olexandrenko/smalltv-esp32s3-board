#pragma once

#include "Decode.h"
#include "libmad/config.h"
#include "libmad/mad.h"

namespace audio
{
    class DecodeMP3 : public Decode
    {
    public:
        DecodeMP3();
        DecodeMP3(void *preallocateSpace, int preallocateSize);
        DecodeMP3(void *buff, int buffSize, void *stream, int streamSize, void *frame, int frameSize, void *synth, int synthSize);
        ~DecodeMP3() override;

        bool begin(Source *source, Output *output) override;
        bool loop() override;
        bool stop() override;
        void desync () override;

        static constexpr int preAllocSize () { return preAllocBuffSize() + preAllocStreamSize() + preAllocFrameSize() + preAllocSynthSize(); }
        static constexpr int preAllocBuffSize () { return ((buffLen + 7) & ~7); }
        static constexpr int preAllocStreamSize () { return ((sizeof(struct mad_stream) + 7) & ~7); }
        static constexpr int preAllocFrameSize () { return (sizeof(struct mad_frame) + 7) & ~7; }
        static constexpr int preAllocSynthSize () { return (sizeof(struct mad_synth) + 7) & ~7; }

    protected:
        void *preallocateSpace = nullptr;
        int preallocateSize = 0;
        void *preallocateStreamSpace = nullptr;
        int preallocateStreamSize = 0;
        void *preallocateFrameSpace = nullptr;
        int preallocateFrameSize = 0;
        void *preallocateSynthSpace = nullptr;
        int preallocateSynthSize = 0;

        static constexpr int buffLen = 0x600; // Slightly larger than largest MP3 frame
        unsigned char *buff;
        int lastReadPos;
        int lastBuffLen;
        unsigned int lastRate;
        int lastChannels;
        
        // Decoding bits
        bool madInitted;
        struct mad_stream *stream;
        struct mad_frame *frame;
        struct mad_synth *synth;
        int samplePtr;
        int nsCount;
        int nsCountMax;

        // The internal helpers
        enum mad_flow ErrorToFlow();
        enum mad_flow Input();
        bool DecodeNextFrame();
        bool GetOneSample(int16_t sample[2]);

    private:
        int unrecoverable = 0;
    };
}
