#pragma once

#include "audio/source/Source.h"
#include "audio/Output.h"

namespace audio
{
    class Decode
    {
    public:
        Decode() : _run(false), _source(nullptr), _output(nullptr), _sample{0} {};
        virtual ~Decode() {};

        virtual bool begin(Source* source, Output* output) { return false; };
        virtual bool loop() { return false; };
        virtual bool stop() { return false; };
        virtual bool isRunning() { return false;};
        virtual void desync () {};

    protected:
        bool _run;
        Source* _source;
        Output* _output;
        int16_t _sample[2];
    };
}