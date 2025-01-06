#pragma once

#include <FS.h>
#include "Source.h"

namespace audio
{
    class SourceMemory : public Source
    {
    public:
        SourceMemory();
        SourceMemory(const void* data, uint32_t size);
        SourceMemory(fs::FS& fs, const char* path);
        virtual ~SourceMemory();

        virtual bool open(const void* data, uint32_t size);
        virtual bool open(fs::FS& fs, const char* path);
        uint32_t read(void* data, uint32_t size) override;
        bool seek(int32_t pos, int dir) override;
        bool close() override;
        bool isOpen() override;
        uint32_t getSize() override;
        uint32_t getPos() override;

    protected:
        bool _alloc;
        void* _data;
        uint32_t _size;
        uint32_t _dptr;
    };
}
