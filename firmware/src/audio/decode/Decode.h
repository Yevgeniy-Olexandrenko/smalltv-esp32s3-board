#pragma once

#include "audio/source/Source.h"
#include "audio/output/Output.h"

namespace audio
{
    class Decode
    {
    public:
        Decode() : _run(false), _source(nullptr), _output(nullptr), _sample{0} {};
        virtual ~Decode() {};

        // Callbacks for status and metadata tracking
        virtual void setCallback(StatusCallback cb, void* data) { _status.setCallback(cb, data); }
        virtual void setCallback(MetadataCallback cb, void* data) { _metadata.setCallback(cb, data); }

        // Default begin behaviour, must be called from descendants
        virtual bool begin(Source* source, Output* output) 
        {
            if (_run && !stop()) return false;
            if (!source || !output) return false;
            _source = source; _source->_status = this->_status; _source->_metadata = this->_metadata;
            _output = output; _output->_status = this->_status;
            return _source->isOpen(); 
        };

        virtual bool loop() { return false; };
        virtual bool stop() { return false; };
        virtual bool isRunning() { return _run; };
        virtual void desync () {};

    protected:
        bool _run;

        Source* _source;
        Output* _output;
        int16_t _sample[2];

        Status _status;
        Metadata _metadata;
    };
}