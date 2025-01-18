#pragma once

#include <stdint.h>
#include "audio/Status.h"
#include "audio/Metadata.h"

namespace audio
{
    class Source
    {
        friend class Decode;

    public:
        Source() {};
        virtual ~Source() {};

        virtual uint32_t read(void* data, uint32_t len) { return 0; };
        virtual uint32_t readNonBlock(void* data, uint32_t len) { return read(data, len); }; 
        virtual bool seek(int32_t pos, int dir) { return false; };
        virtual bool close() { return false; };
        virtual bool isOpen() { return false; };
        virtual uint32_t getSize() { return 0; };
        virtual uint32_t getPos() { return 0; };
        virtual bool loop() { return true; };

    protected:
        Status _status;
        Metadata _metadata;
    };
}
