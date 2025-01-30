#pragma once

#include "Source.h"

namespace audio
{
    class SourceExtractID3 : public Source
    {
    public:
        SourceExtractID3(Source* source) : _source(source), _checked(false) {}
        ~SourceExtractID3() override {}
        
        uint32_t read(void* data, uint32_t len) override;
        bool seek(int32_t pos, int dir) override { return _source->seek(pos, dir); }
        bool close() override { return _source->close(); }
        bool isOpen() override { return _source->isOpen(); }
        uint32_t getSize() override { return _source->getSize(); }
        uint32_t getPos() override { return _source->getPos(); }

    private:
        Source* _source;
        bool _checked;
    };
}
