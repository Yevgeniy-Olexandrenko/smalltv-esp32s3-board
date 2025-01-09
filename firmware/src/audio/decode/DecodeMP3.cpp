#pragma GCC optimize ("O3")

// #include <stdio.h>
#include <memory.h>
#include "DecodeMP3.h"

namespace audio
{
    DecodeMP3::DecodeMP3()
    {
        // running = false;
        // file = NULL;
        // output = NULL;
        buff = nullptr;
        synth = nullptr;
        frame = nullptr;
        stream = nullptr;
        nsCountMax = 1152/32;
        madInitted = false;
    }

    DecodeMP3::DecodeMP3(void *space, int size)
        : preallocateSpace(space), preallocateSize(size)
    {
        // running = false;
        // file = NULL;
        // output = NULL;
        buff = nullptr;
        synth = nullptr;
        frame = nullptr;
        stream = nullptr;
        nsCountMax = 1152/32;
        madInitted = false;
    }

    DecodeMP3::DecodeMP3(void *buff, int buffSize, void *stream, int streamSize, void *frame, int frameSize, void *synth, int synthSize)
        : preallocateSpace(buff)
        , preallocateSize(buffSize)
        , preallocateStreamSpace(stream)
        , preallocateStreamSize(streamSize)
        , preallocateFrameSpace(frame)
        , preallocateFrameSize(frameSize)
        , preallocateSynthSpace(synth)
        , preallocateSynthSize(synthSize)
    {
        // running = false;
        // file = NULL;
        // output = NULL;
        buff = nullptr;
        synth = nullptr;
        frame = nullptr;
        stream = nullptr;
        nsCountMax = 1152/32;
        madInitted = false;
    }

    DecodeMP3::~DecodeMP3()
    {
       if (!preallocateSpace) 
       {
            free(buff);
            free(synth);
            free(frame);
            free(stream);
        }
    }

    bool DecodeMP3::begin(Source *__source, Output *__output)
    {
        if (!__source) return false;
        _source = __source;
        if (!__output) return false;
        _output = __output;

        if (!_source->isOpen()) 
        {
        //@@    audioLogger->printf_P(PSTR("MP3 source file not open\n"));
            return false; // Error
        }

        // Reset error count from previous file
        unrecoverable = 0;

        _output->SetBitsPerSample(16); // Constant for MP3 decoder
        _output->SetChannels(2);

        if (!_output->begin()) return false;

        // Where we are in generating one frame's data, set to invalid so we will run loop on first getsample()
        samplePtr = 9999;
        nsCount = 9999;
        lastRate = 0;
        lastChannels = 0;
        lastReadPos = 0;
        lastBuffLen = 0;

        // Allocate all large memory chunks
        if (preallocateStreamSize + preallocateFrameSize + preallocateSynthSize) 
        {
            if (preallocateSize >= preAllocBuffSize() &&
                preallocateStreamSize >= preAllocStreamSize() &&
                preallocateFrameSize >= preAllocFrameSize() &&
                preallocateSynthSize >= preAllocSynthSize()) 
            {
                buff = reinterpret_cast<unsigned char *>(preallocateSpace);
                stream = reinterpret_cast<struct mad_stream *>(preallocateStreamSpace);
                frame = reinterpret_cast<struct mad_frame *>(preallocateFrameSpace);
                synth = reinterpret_cast<struct mad_synth *>(preallocateSynthSpace);
            }
            else 
            {
                _output->stop();
            //@@    audioLogger->printf_P("OOM error in MP3:  Want %d/%d/%d/%d bytes, have %d/%d/%d/%d bytes preallocated.\n",
            //@@        preAllocBuffSize(), preAllocStreamSize(), preAllocFrameSize(), preAllocSynthSize(),
            //@@        preallocateSize, preallocateStreamSize, preallocateFrameSize, preallocateSynthSize);
            return false;
            }
        } 
        else if (preallocateSpace) 
        {
            uint8_t *p = reinterpret_cast<uint8_t *>(preallocateSpace);
            buff = reinterpret_cast<unsigned char *>(p);
            p += preAllocBuffSize();
            stream = reinterpret_cast<struct mad_stream *>(p);
            p += preAllocStreamSize();
            frame = reinterpret_cast<struct mad_frame *>(p);
            p += preAllocFrameSize();
            synth = reinterpret_cast<struct mad_synth *>(p);
            p += preAllocSynthSize();
            int neededBytes = p - reinterpret_cast<uint8_t *>(preallocateSpace);
            if (neededBytes > preallocateSize) 
            {
                _output->stop();
            //@@    audioLogger->printf_P("OOM error in MP3:  Want %d bytes, have %d bytes preallocated.\n", neededBytes, preallocateSize);
                return false;
            }
        } 
        else 
        {
            buff = reinterpret_cast<unsigned char *>(malloc(buffLen));
            stream = reinterpret_cast<struct mad_stream *>(malloc(sizeof(struct mad_stream)));
            frame = reinterpret_cast<struct mad_frame *>(malloc(sizeof(struct mad_frame)));
            synth = reinterpret_cast<struct mad_synth *>(malloc(sizeof(struct mad_synth)));
            if (!buff || !stream || !frame || !synth) 
            {
                free(buff);
                free(stream);
                free(frame);
                free(synth);

                buff = nullptr;
                stream = nullptr;
                frame = nullptr;
                synth = nullptr;

                _output->stop();
            //@@    audioLogger->printf_P("OOM error in MP3\n");
                return false;
            }
        }

        mad_stream_init(stream);
        mad_frame_init(frame);
        mad_synth_init(synth);
        synth->pcm.length = 0;
        mad_stream_options(stream, 0); // TODO - add options support
        madInitted = true;

        _run = true;
        return true;
    }

    bool DecodeMP3::loop()
    {
        if (!_run) goto done; // Nothing to do here!

        // First, try and push in the stored sample.  If we can't, then punt and try later
        if (!_output->ConsumeSample(_sample)) goto done; // Can't send, but no error detected

        // Try and stuff the buffer one sample at a time
        do
        {
            // Decode next frame if we're beyond the existing generated data
            if (samplePtr >= synth->pcm.length && nsCount >= nsCountMax)
            {
            retry:
                if (Input() == MAD_FLOW_STOP) 
                {
                    return false;
                }

                if (!DecodeNextFrame()) 
                {
                    if (stream->error == MAD_ERROR_BUFLEN) 
                    {
                        // randomly seeking can lead to endless
                        // and unrecoverable "MAD_ERROR_BUFLEN" loop
                    //@@    audioLogger->printf_P(PSTR("MP3:ERROR_BUFLEN %d\n"), unrecoverable);
                        if (++unrecoverable >= 3) 
                        {
                            unrecoverable = 0;
                            stop();
                            return _run;
                        }
                    } 
                    else 
                    {
                        unrecoverable = 0;
                    }
                    goto retry;
                }
                samplePtr = 9999;
                nsCount = 0;
            }

            if (!GetOneSample(_sample)) 
            {
            //@@    audioLogger->printf_P(PSTR("G1S failed\n"));
                _run = false;
                goto done;
            }
            if (lastChannels == 1)
            {
                _sample[1] = _sample[0];
            }
        } 
        while (_run && _output->ConsumeSample(_sample));

    done:
        _source->loop();
        _output->loop();
        return _run;
    }

    bool DecodeMP3::stop()
    {
        if (madInitted) 
        {
            mad_synth_finish(synth);
            mad_frame_finish(frame);
            mad_stream_finish(stream);
            madInitted = false;
        }

        if (!preallocateSpace) 
        {
            free(buff);
            free(synth);
            free(frame);
            free(stream);
        }

        buff = nullptr;
        synth = nullptr;
        frame = nullptr;
        stream = nullptr;

        _run = false;
        _output->stop();
        return _source->close();
    }
    
    void DecodeMP3::desync()
    {
    //@@    audioLogger->printf_P(PSTR("MP3:desync\n"));
        if (stream) 
        {
            stream->next_frame = nullptr;
            stream->this_frame = nullptr;
            stream->sync = 0;
        }
        lastBuffLen = 0;
    }

    mad_flow DecodeMP3::ErrorToFlow()
    {
        char err[64];
        char errLine[128];

        // Special case - eat "lost sync @ byte 0" as it always occurs and is not really correct....it never had sync!
        if (lastReadPos == 0 && stream->error == MAD_ERROR_LOSTSYNC) return MAD_FLOW_CONTINUE;

    //@@    strcpy_P(err, mad_stream_errorstr(stream));
    //@@    snprintf_P(errLine, sizeof(errLine), PSTR("Decoding error '%s' at byte offset %d"),
    //@@            err, (stream->this_frame - buff) + lastReadPos);
    //@@    yield(); // Something bad happened anyway, ensure WiFi gets some time, too
    //@@    cb.st(stream->error, errLine);
        return MAD_FLOW_CONTINUE;
    }

    mad_flow DecodeMP3::Input()
    {
        int unused = 0;
        if (stream->next_frame) 
        {
            unused = lastBuffLen - (stream->next_frame - buff);
            if (unused < 0) 
            {
                desync();
                unused = 0;
            } 
            else 
            {
                memmove(buff, stream->next_frame, unused);
            }
            stream->next_frame = NULL;
        }

        if (unused == lastBuffLen) 
        {
            // Something wicked this way came, throw it all out and try again
            unused = 0;
        }

        lastReadPos = (_source->getPos() - unused);
        int len = (buffLen - unused);
        len = _source->read(buff + unused, len);
        if (len == 0 && unused == 0)
        {
            // Can't read any from the file, and we don't have anything left.  It's done....
            return MAD_FLOW_STOP;
        }
        if (len < 0)
        {
            desync();
            unused = 0;
        }

        lastBuffLen = len + unused;
        mad_stream_buffer(stream, buff, lastBuffLen);
        return MAD_FLOW_CONTINUE;
    }

    bool DecodeMP3::DecodeNextFrame()
    {
        if (mad_frame_decode(frame, stream) == -1) 
        {
            ErrorToFlow(); // Always returns CONTINUE
            return false;
        }
        nsCountMax  = MAD_NSBSAMPLES(&frame->header);
        return true;
    }

    bool DecodeMP3::GetOneSample(int16_t sample[2])
    {
        if (synth->pcm.samplerate != lastRate) 
        {
            _output->SetRate(synth->pcm.samplerate);
            lastRate = synth->pcm.samplerate;
        }
        if (synth->pcm.channels != lastChannels) 
        {
            _output->SetChannels(synth->pcm.channels);
            lastChannels = synth->pcm.channels;
        }

        // If we're here, we have one decoded frame and sent 0 or more samples out
        if (samplePtr < synth->pcm.length) 
        {
            sample[Output::LEFTCHANNEL ] = synth->pcm.samples[0][samplePtr];
            sample[Output::RIGHTCHANNEL] = synth->pcm.samples[1][samplePtr];
            samplePtr++;
        } 
        else 
        {
            samplePtr = 0;
            switch ( mad_synth_frame_onens(synth, frame, nsCount++) ) 
            {
                case MAD_FLOW_STOP:
                case MAD_FLOW_BREAK: 
                //@@    audioLogger->printf_P(PSTR("msf1ns failed\n"));
                    return false; // Either way we're done
                default:
                    break; // Do nothing
            }
            // for IGNORE and CONTINUE, just play what we have now
            sample[Output::LEFTCHANNEL ] = synth->pcm.samples[0][samplePtr];
            sample[Output::RIGHTCHANNEL] = synth->pcm.samples[1][samplePtr];
            samplePtr++;
        }
        return true;
    }

    // The following are helper routines for use in libmad to check stack/heap free
    // and to determine if there's enough stack space to allocate some blocks there
    // instead of precious heap.

    #undef stack
    extern "C" {
    #ifdef ESP32
    //TODO - add ESP32 checks
    void stack(const char *s, const char *t, int i)
    {
    }
    int stackfree()
    {
        return 8192;
    }
    }
    #endif
}
