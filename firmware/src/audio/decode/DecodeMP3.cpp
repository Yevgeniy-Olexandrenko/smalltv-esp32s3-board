#pragma GCC optimize ("O3")

#include <stdio.h>
#include <memory.h>
#include "DecodeMP3.h"

namespace audio
{
    DecodeMP3::DecodeMP3()
    {
        hMP3Decoder = MP3InitDecoder();
        if (!hMP3Decoder) 
        {
            // audioLogger->printf_P(PSTR("Out of memory error! hMP3Decoder==NULL\n"));
            // Serial.flush();
        }

        // For sanity's sake...
        memset(buff, 0, sizeof(buff));
        memset(outSample, 0, sizeof(outSample));
        buffValid = 0;
        lastFrameEnd = 0;
        validSamples = 0;
        curSample = 0;
        lastRate = 0;
        lastChannels = 0;
    }

    DecodeMP3::~DecodeMP3()
    {
        MP3FreeDecoder(hMP3Decoder);
    }

    bool DecodeMP3::begin(Source* source, Output* output)
    {
        if (!source) return false;
        _source = source;
        if (!output) return false;
        _output = output;
        if (!_source->isOpen()) return false; // Error

        _output->begin();
        _output->SetBitsPerSample(16);
  
        _run = true;
        return true;
    }

    bool DecodeMP3::loop()
    {
        if (!_run) goto done; // Nothing to do here!

        // If we've got data, try and pump it out...
        while (validSamples) 
        {
            _sample[0] = outSample[curSample * 2 + 0];
            _sample[1] = outSample[curSample * 2 + 1];
            if (!_output->ConsumeSample(_sample)) goto done; // Can't send, but no error detected
            validSamples--;
            curSample++;
        }

        // No samples available, need to decode a new frame
        if (FillBufferWithValidFrame()) 
        {
            // buff[0] start of frame, decode it...
            unsigned char *inBuff = reinterpret_cast<unsigned char *>(buff);
            int bytesLeft = buffValid;
            int ret = MP3Decode(hMP3Decoder, &inBuff, &bytesLeft, outSample, 0);
            if (ret) 
            {
                // Error, skip the frame...
                // char buff[48];
                // sprintf(buff, "MP3 decode error %d", ret);
                // cb.st(ret, buff);
            } 
            else 
            {
                lastFrameEnd = buffValid - bytesLeft;
                MP3FrameInfo fi;
                MP3GetLastFrameInfo(hMP3Decoder, &fi);
                if ((int)fi.samprate != (int)lastRate) 
                {
                    _output->SetRate(fi.samprate);
                    lastRate = fi.samprate;
                }
                if (fi.nChans != lastChannels) 
                {
                    _output->SetChannels(fi.nChans);
                    lastChannels = fi.nChans;
                }
                curSample = 0;
                validSamples = fi.outputSamps / lastChannels;
            }
        } 
        else 
        {
            _run = false; // No more data, we're done here...
        }

    done:
        _source->loop();
        _output->loop();
        return _run;
    }

    bool DecodeMP3::stop()
    {
        if (_run)
        {
            _run = false;
            _output->stop();
            return _source->close();
        }
        return true;
    }

    bool DecodeMP3::FillBufferWithValidFrame()
    {
        buff[0] = 0; // Destroy any existing sync word @ 0
        int nextSync;
        do 
        {
            nextSync = MP3FindSyncWord(buff + lastFrameEnd, buffValid - lastFrameEnd);
            if (nextSync >= 0) nextSync += lastFrameEnd;
            lastFrameEnd = 0;
            if (nextSync == -1) 
            {
                // Could be 1st half of syncword, preserve it...
                if (buff[buffValid-1] == 0xff) 
                { 
                    buff[0] = 0xff;
                    buffValid = _source->read(buff + 1, sizeof(buff) - 1);
                    if (buffValid == 0) return false; // No data available, EOF
                }

                // Try a whole new buffer
                else
                {
                    buffValid = _source->read(buff, sizeof(buff));
                    if (buffValid == 0) return false; // No data available, EOF
                }
            }
        } while (nextSync == -1);

        // Move the frame to start at offset 0 in the buffer
        buffValid -= nextSync; // Throw out prior to nextSync
        memmove(buff, buff + nextSync, buffValid);

        // We have a sync word at 0 now, try and fill remainder of buffer
        buffValid += _source->read(buff + buffValid, sizeof(buff) - buffValid);
        return true;
    }
}
