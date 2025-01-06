#include <unistd.h>
#include <memory.h>
#include "SourceMemory.h"

namespace audio
{
    SourceMemory::SourceMemory()
        : _alloc(false)
        , _data(nullptr)
        , _size(0)
        , _dptr(0)
    {}

    SourceMemory::SourceMemory(const void* data, uint32_t size)
    {
        open(data, size);
    }

    SourceMemory::SourceMemory(fs::FS &fs, const char *path)
    {
        open(fs, path);
    }

    SourceMemory::~SourceMemory()
    {
        close();
    }

    bool SourceMemory::open(const void* data, uint32_t size)
    {
        if (data && size > 0)
        {
            _data = const_cast<void*>(data);
            _size = size;
            _dptr = 0;
        }
        return bool(_data);
    }

    bool SourceMemory::open(fs::FS &fs, const char *path)
    {
        if (path)
        {
            fs::File file = fs.open(path, "r");
            if (auto size = file.size())
            {
                if (auto data = ps_malloc(size))
                {
                    if (file.readBytes(reinterpret_cast<char*>(data), size) == size)
                    {
                        _alloc = true;
                        _data  = data;
                        _size  = size;
                        _dptr  = 0;
                    } else free(data);
                }
            }
            file.close();
        }
        return bool(_data);
    }

    uint32_t SourceMemory::read(void* data, uint32_t size)
    {
        if (_data && _dptr < _size)
        {
            uint32_t read = (_size - _dptr);
            if (read > size) read = size;
            memcpy(data, reinterpret_cast<const uint8_t*>(_data) + _dptr, read);
            _dptr += read;
            return read;
        }
        return 0;  
    }

    bool SourceMemory::seek(int32_t pos, int dir)
    {
        if (_data)
        {
            uint32_t dptr;
            switch (dir) 
            {
                case SEEK_SET: dptr = pos; break;
                case SEEK_CUR: dptr = _dptr + pos; break;
                case SEEK_END: dptr = _size - pos; break;
                default: return false;
            }
            if (dptr <= _size)
            {
                _dptr = dptr;
                return true;
            }
        }
        return false;
    }

    bool SourceMemory::close()
    {
        if (_alloc) free(_data);
        _data = nullptr;
        _size = 0;
        _dptr = 0;
        return true;
    }

    bool SourceMemory::isOpen()
    {
        return bool(_data);
    }

    uint32_t SourceMemory::getSize()
    {
        return (_data ? _size : 0);
    }

    uint32_t SourceMemory::getPos()
    {
        return (_data ? _dptr : 0);
    }
}
