#pragma once

#include <FS.h>
#include "Source.h"

namespace audio
{
    class SourceFile : public Source
    {
    public:
        SourceFile(fs::FS& fs);
        SourceFile(fs::FS& fs, const char* path);
        ~SourceFile() override;
        
        virtual bool open(const char* path);
        uint32_t read(void* data, uint32_t size) override;
        bool seek(int32_t pos, int dir) override;
        bool close() override;
        bool isOpen() override;
        uint32_t getSize() override;
        uint32_t getPos() override;

    protected:
        fs::FS* _fs;
        fs::File _file;
    };
}