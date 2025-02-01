#include "SourceFile.h"

namespace audio
{
    SourceFile::SourceFile()
    {}

    SourceFile::SourceFile(fs::FS &fs, const char *path)
    {
        open(fs, path);
    }

    SourceFile::~SourceFile()
    {
        close();
    }

    bool SourceFile::open(fs::FS &fs, const char *path)
    {
        if (!_file && path)
        {
            _file = fs.open(path, "r");
        }
        return _file;
    }

    uint32_t SourceFile::read(void *data, uint32_t size)
    {
        return _file.read(reinterpret_cast<uint8_t*>(data), size);
    }

    bool SourceFile::seek(int32_t pos, int dir)
    {
        return _file.seek(pos, dir == SEEK_SET ? SeekSet : (dir == SEEK_CUR ? SeekCur : SeekEnd));
    }

    bool SourceFile::close()
    {
        _file.close();
        return true;
    }

    bool SourceFile::isOpen()
    {
        return _file;
    }

    uint32_t SourceFile::getSize()
    {
        return (_file ? _file.size() : 0);
    }

    uint32_t SourceFile::getPos()
    {
        return (_file ? _file.position() : 0);
    }
}
